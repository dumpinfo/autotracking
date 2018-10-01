     /*****************************************************************************
 * File:	Main.c
 * Desc:	Main functions of CTracker TestBed
 * Author:	Xuhui Zhou  @ Carnegie Mellon University
 * Date:	11/13/2003
 *****************************************************************************/

#include <windows.h>
#include <winuser.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <commdlg.h>
#include <time.h>
#include "pxc.h"
#include "resource.h"
#include "math.h"
#include <direct.h>
#include "new.h"
#include "utility.h"
#include "ImageIO.h"
#include "MeanShiftWrap.h"
#include "KltWrap.h"
#include "Histogram.h"
#include "TemplateMatch.h"
#include "Msfeature.h"
#include "Emshift.h"
#include "GraphCutTracker.h"
#include "ParticleFilter.h"

/* this program can use either 24 bit color or 8 bit gray frames.
  change the value of PIXEL_TYPE to PBITS_RGB24 or PBITS_Y8 */
#define PBITS_Y8 			0x0008
#define PBITS_RGB24			0x0218
#define PIXEL_TYPE PBITS_RGB24

#ifdef _WIN32
#define PXC_NAME "pxc_95.dll"
#define FRAME_NAME "frame_32.dll"
#define PXC_NT "pxc_nt.dll"
#else
#define PXC_NAME "pxc_31.dll"
#define FRAME_NAME "frame_16.dll"
#define PXC_NT    "pxc_31.dll"
#endif

#define TEST_TIMER 1
#define ON  1
#define OFF 2

  /*-------------------------------------------------------------------------
   This button structure is used to create the function buttons as child
   windows. There are other ways to do this. We figured this would be the
   best way to do it for our example.
   -------------------------------------------------------------------------*/
#define NBUTTONS 12
typedef struct tagBUTTON
{
	HWND hwnd;
	long style;
	LPSTR text;
	int id;
} BUTTON;

  /*-------------------------------------------------------------------------
  These values are used to control the processing of the WM_COMMAND message.
   -------------------------------------------------------------------------*/
#define ID_FBASE		101
#define ID_TRACKFWD		ID_FBASE+0
#define ID_STOP			ID_FBASE+1
#define ID_BROWSEBACK   ID_FBASE+2
#define ID_STEPFWD      ID_FBASE+3
#define ID_REWIND		ID_FBASE+4
#define ID_PREDICT		ID_FBASE+5
#define ID_READ			ID_FBASE+6
#define ID_PLAYLOG		ID_FBASE+7
#define ID_KLT		    ID_FBASE+8
#define ID_LOG		    ID_FBASE+9
#define ID_ALGORITHM	ID_FBASE+10
#define ID_DEBUG		ID_FBASE+11

BUTTON Functions[NBUTTONS] =
{		
	{NULL, BS_PUSHBUTTON, "Open", ID_READ},
	{NULL, BS_PUSHBUTTON, "Algorithm", ID_ALGORITHM},
	{NULL, BS_PUSHBUTTON, "<<Browse ", ID_BROWSEBACK},	
	{NULL, BS_PUSHBUTTON, "Track>>", ID_TRACKFWD},		
	{NULL, BS_PUSHBUTTON, "Step>", ID_STEPFWD},		
	{NULL, BS_PUSHBUTTON, "Stop", ID_STOP},	
	{NULL, BS_PUSHBUTTON, "Rewind", ID_REWIND},			
	{NULL, BS_PUSHBUTTON, "Klt", ID_KLT},		
	{NULL, BS_PUSHBUTTON, "Log", ID_LOG},
	{NULL, BS_PUSHBUTTON, "Rpt", ID_PLAYLOG},	
	{NULL, BS_PUSHBUTTON, "Predict", ID_PREDICT},	
	{NULL, BS_PUSHBUTTON, "Debug", ID_DEBUG}	
};

//second set on debug win
BUTTON Functions2[NBUTTONS] =
{	
	{NULL, BS_PUSHBUTTON, " Open ", ID_READ},	
	{NULL, BS_PUSHBUTTON, " Algorithm ", ID_ALGORITHM},
	{NULL, BS_PUSHBUTTON, " <<Browse ", ID_BROWSEBACK},	
	{NULL, BS_PUSHBUTTON, " Track>> ", ID_TRACKFWD},		
	{NULL, BS_PUSHBUTTON, " Step> ", ID_STEPFWD},	
	{NULL, BS_PUSHBUTTON, " Stop ", ID_STOP},	
	{NULL, BS_PUSHBUTTON, " Rewind ", ID_REWIND},			
	{NULL, BS_PUSHBUTTON, " Klt ", ID_KLT},			
	{NULL, BS_PUSHBUTTON, " Log ", ID_LOG},	
	{NULL, BS_PUSHBUTTON, " Rpt ", ID_PLAYLOG},
	{NULL, BS_PUSHBUTTON, " Predict ", ID_PREDICT},	
	{NULL, BS_PUSHBUTTON, "Debug", ID_DEBUG}	
};

static unsigned ButtonTextLen;
static int ButtonHeight, ButtonWidth;
static int ViewOffset;

BOOL AppInit(HINSTANCE hInst, HINSTANCE hPrev, LPSTR szCmdLine, int sw);
LONG WINAPI CtlProc(HWND, UINT, WPARAM, LPARAM);
HWND CreateControlWindow(HINSTANCE hInst, LPSTR szCmdLine, int sw);
void RegisterWindowClasses(HINSTANCE hInst, LPSTR szCmdLine, int sw);
void AppExit(void);
BOOL AppIdle(void);
void AppPaint(HWND hwnd, HDC hdc);
void AppPaint2(HWND hwnd, HDC hdc);
void GetGlobals(HWND hwnd, LPARAM lParam);
int GetImage();
void DrawButtons(HWND hwnd, LPARAM lParam, int draw);
void DrawButtons2(HWND hwnd, LPARAM lParam, int draw);
void SetBitMapHead(void);
int AllocBuffer(void);
void ReleaseBuffer();
void HourGlass(int on);

  /* text data for drawing text */
static int cxChar,                               /* average character width */
           cyChar;                                      /* character height */

static HINSTANCE hAppInst;
static HWND hwndCtl;
static HWND hwndCtl2;
static char szAppName[] = "PXCDRAW2" ;
static char szTitleBar[100];
static char szTitle[] = "DHID";
static int  iBoardRev;

static char szVideoType[80];
static int videotype;
static int modelnumber;
static int grab_type;
//static BOOL fAcquire;

static int  ImageMaxX,
            ImageMaxY,
            WindowX,
            WindowY;

static int  StartTest,
            FrameCount,
            ErrorCount;
/*-------------------------------< Time Data >-------------------------------*/
double bTime, eTime, dTime, iTime;
double bTime2,dTime2;
int FPS;
double dFPS;


/*----------------------------< GDI Functions >------------------------------*/
void CreateGrayPalette(void);

static HPALETTE hpalette;
static HANDLE hBuf;
static BYTE __PX_HW *gpBits;
static HANDLE hBuf2;	//for previous image in history
static BYTE __PX_HW *gpBits2; 

struct {
  BITMAPINFOHEADER head;
  RGBQUAD colors[256];
} maphead;

/*-------------------------------< file data >-------------------------------*/
static OPENFILENAME rfn, sfn;
static char szFilterSel[] = "(*.bmp,*.jpg)\0*.bmp;*.jpg\0";

static char tmp[201];
static char *WriteErr = "Stop acquiring an image before writing a file";
static char *ReadErr = "Stop acquiring an image before reading a file";

//-------------------------------<Global Definition >-------------------------------//
void	dis_PrintMessage(); //display message as window title

//-------------------------------<Image IO and Sequence playing >-------------------------------//
//variables for sequence playing
IplImage	*gcImgCurrent=NULL;
IplImage	*gcImgCurrentHalf=NULL;
int			giFrameInitial = 1;
int			giPlayFlag = 0;		//flag to play sequence

//Image IO Related Method
void		AppSetDisplay();
int			img_StepOneFrame(int nextFrameNo);
void		dis_ShowImgCurrent();
void		dis_ShowImgDebug();

//-------------------------------<Track Click Initialization>-------------------------------//
#define		NUMPOINTS 4
POINT		gaClickPoint[NUMPOINTS];		//bmp image corrdinate (bottom-left is 0)
POINT		gaClickPointDisplay[NUMPOINTS]; //display coordinate (top-left is 0)
POINT		gaClickPointHalf[NUMPOINTS];	//half coordinate (top-left is 0)
POINT		gaClickPointHalfDisplay[NUMPOINTS];	//half coordinate (bottom-left is 0)
int			giClickCount=0;
BOOL		gbClickInitFlag=FALSE;
RECT		gRectInit;
RECT		gRectCurrent;
RECT		gRectCurrentHalf;

//poly region handle
HRGN		gRegionInit;
HRGN		gRegionCurrent;
HRGN		gRegionCurrentHalf;
HRGN		gRegionDisplay;
HRGN		gRegionDisplayHalf;

//Click related methods
void		dis_DrawRect(HWND hwnd, RECT rect);
void		img_ProcessLClick_Poly();
void		dis_DrawRegion(HWND hwnd, HRGN hRegion);
void		img_ProcessMouseMove(long x, long y);

//-------------------------------<MeanShift Tracking>-------------------------------//
int			giTrackHalf = 1;
BOOL		gbHalfSize = TRUE;
BOOL		gbTrackMood = FALSE;

//-------------------------------<KLT Tracking>-------------------------------//
float		gfXShift, gfYShift;
int			giCountValid, giCountBG;

//-------------------------------<Histogram Projection>-------------------------------//
extern		RBINS;
extern		GBINS;
extern		BBINS;

//-------------------------------<Tracker Report>-------------------------------//
typedef struct {
	double time;
	char	*framePath;	
	double	vx;
	double	vy;
	double	aspectAng;
	double	grazingAng;
	double	resolution;
	int		boxX;
	int		boxY;
	int		boxWid;
	int		boxHgt;	
	int		imgWidth;
	int		imgHeight;
}TrackReport;
TrackReport tReport;
char		gLogDir[_MAX_PATH];		//log dir path
char		gLogDirName[_MAX_DIR];	//log dir name
char		gLogFile[_MAX_PATH];
BOOL		gLogDirFlag = FALSE;	//build a sub-directory for one tracking click
char		gClickFrameTitle[100];
char		gLastFramePath[_MAX_PATH]; //record previous frame path
IplImage*	gImgBitMap;
char		gExePath[_MAX_PATH];
char		gExeLogPath[_MAX_PATH];
int			gLogVersion=5; //1: CMU-v1.0; 2: CID-v0.9; 3: CID-v1.0; 4:CMU V2.0;; 5: cmu v2.1

//methods
void		io_PrintReport();		//version 1: CMU version 1.0
void		io_PrintReport_CID09();	//version 2: CID version 0.9
void		io_PrintReport_CID10();	//version 3: CID version 1.0
void		io_PrintReport_V20();	//version 4: CMU version 2.0
void		io_PrintReport_V21();	//version 5: CMU version 2.1
//-------------------------------<Tracker Report>-------------------------------//
BOOL		gbReplayFlag=FALSE;
BOOL		gbKltFlag = FALSE;
int			giSeqType = 0; //0:image; 1: trk
BOOL		gbLogFlag = FALSE;

//-------------------------------<Display>-------------------------------//
COLORREF	crPenColor = RGB(255,0,0);
int			giPenWidth = 3;

//-------------------------------<Prediction>-------------------------------//
#define		POSNUM 40
//general type
typedef struct _FPOINT{
    float  x;
    float  y;
} 
FPOINT;

FPOINT		gPosPredict[POSNUM];	//prediction array
POINT		gPosDraw[POSNUM];		//flipped prediction array for draw in sreen window
//POINT		gPosTarget;				//target position
int			gPosCount = 0;
int			gEffectNum = POSNUM;
float		gAvgDx, gAvgDy;
BOOL		gbPredictFlag = FALSE;
int			gBlobWidthInit, gBlobHeightInit; //initial dimension of tracking blob
int			gBlobWidth, gBlobHeight; //blob dimension of previous frame
int			gIgnoreFrameNum = 8;		//occlusion detection delay, recent number of frame to ignore.
BOOL		gbAutoPredictFlag = FALSE;
float		gAffineMatrix[6];
float		gZoomAccum = 1; //accumulative scale factor
int			gTrackCount = 0;

int			img_PredictMotion();
void		dis_DrawTraceLine(HWND hwnd, FPOINT* pts, int ptCount, int penStyle, int penWidth, COLORREF penColor);

//char		sLogDirName[80]; // receives name of log directory. 
BOOL CALLBACK dlg_LogDirProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK dlg_DirExistProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL		dlg_LogDir();
void		io_CalcLogDirName(char* outLogDirName);

//-------------------------------<Batch Process>-------------------------------//
BOOL		gBatchFlag;
extern int	gBtkFileCount;
extern char	gBtkFileArray[100][MAX_PATH];
extern int	gBtkStepNum[100];
void		BatchProcess();
char		gLogPathBackup[MAX_PATH];
char		gBtkPath[MAX_PATH];	
int			gTrkIdx;
int			gStepCount;
BOOL		gbBatchStopFlag;

BOOL CALLBACK dlg_BatchConfirmProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK dlg_BatchFinishProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
void Batch_InitTrkFile(char *sTrkFile);
void Batch_Init();
void Batch_Exit(boolean flag);

//-------------------------------<TestBed>-------------------------------//
#define 	DEBUG 0
int			gRbins=10;
int			gGbins=10;
int			gBbins=10;
BOOL		gbBinChange = FALSE;
int			gTrackerSelect;
int			gTrackerIndex=3;	//default tracker algorithm 
BOOL		gDebugWinFlag = FALSE; //whether to display second debug window
IplImage	*gImgDisplayMain;
IplImage	*gImgDisplayDebug;
IplImage	*gImgInitMaskHalf=NULL;
IplImage	*gImgInitMask=NULL;
TkResult	gTkResult;

extern KLT_FeatureTable	ftBg;
void		dis_DrawKltBgLines();

void		img_CreateRegionMask(HRGN inRegion, IplImage *outImgMask);
BOOL		dlg_Option();
BOOL CALLBACK dlg_OptionProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
void		img_RewindVideo();
/***************************************************************************************************************************
 * Name:         WinMain                                                     *
 * Description:  Main window program and message processing loop.            *
 ***************************************************************************************************************************/
int PASCAL WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR szCmdLine, int sw)
{
	MSG msg;
	
	hAppInst = hInst;     /* save for later */

	/* Call initialization procedure */
	if(!AppInit(hInst, hPrev, szCmdLine, sw))
	{
		AppExit();
		return FALSE;
	}

	/*-------------------------------------------------------------------------
	Polling messages from event queue
	-------------------------------------------------------------------------*/
	for(;;)
	{
		if(PeekMessage(&msg, NULL, 0, 0,PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			if(AppIdle())
				WaitMessage();
		}
	}

	AppExit();
	return msg.wParam;
}
/*****************************************************************************
 * Name:         AppInit                                                     *
 *****************************************************************************/
BOOL AppInit(HINSTANCE hInst, HINSTANCE hPrev, LPSTR szCmdLine, int sw)
{
	int rstOpenFile;

	bTime = 0.0;
	FrameCount = 0;
	hBuf = NULL;
	hBuf2 = NULL;

	ImageMaxX = 640;
	ImageMaxY = 480;
	
	//get current path for log files path
	_getcwd(gExePath, _MAX_PATH);
	strcpy(gExeLogPath, gExePath);
	strcat(gExeLogPath, "\\tkLog");
	_mkdir(gExeLogPath);
	
	/* create windows */
	RegisterWindowClasses(hInst, szCmdLine, sw);
	
	hwndCtl = CreateControlWindow(hInst, szCmdLine, sw);
	if(!hwndCtl)
	{
		MessageBox(0, "Cannot create window", szAppName, MB_OK);
		return FALSE;
	}
	ShowWindow(hwndCtl,sw);	
	//draw buttons
	DrawButtons(hwndCtl, 0, TRUE);	

	//read histogram bins setting from file
	his_ReadHistBinsFromFile();

	//read in image files	
	iio_Init(hwndCtl);	


	//prompt user to open file
	rstOpenFile = iio_OpenFileByPrompt();			
	
	//create debug window
	hwndCtl2 = CreateControlWindow(hInst, szCmdLine, sw);
	if(!hwndCtl2)
	{
	MessageBox(0, "Cannot create window", szAppName, MB_OK);
	return FALSE;
	}				
	DrawButtons2(hwndCtl2, 0, TRUE);

	//whether to display this window
	if (gDebugWinFlag==TRUE)
		ShowWindow(hwndCtl2,sw);
	else
		ShowWindow(hwndCtl2,SW_HIDE);
		
	SetTimer(hwndCtl, TEST_TIMER, 1000, NULL);    /* 2-second timer */
	
	//initial display
	ImageMaxX = iio_ImageMaxX();
	ImageMaxY = iio_ImageMaxY();
	gcImgCurrent = iio_GetInputImage();
	giFrameInitial = iio_GetFrameNo();

	//set window dimension and related parameters
	AppSetDisplay();	

	//mean shift track image with half size
	gcImgCurrentHalf = cvCreateImage(cvSize(ImageMaxX/2, ImageMaxY/2), 8, 3);
	//mask image
	gImgInitMaskHalf = cvCreateImage(cvSize(ImageMaxX/2, ImageMaxY/2), 8, 1);
	gImgInitMask = cvCreateImage(cvSize(ImageMaxX, ImageMaxY), 8, 1);
	//bitmap image
	gImgBitMap = cvCreateImage(cvSize(ImageMaxX, ImageMaxY), 8, 1);

	//get half image for processing
	if (gcImgCurrent!=NULL){	
		cvResize(gcImgCurrent, gcImgCurrentHalf, CV_INTER_LINEAR);
	}
	
	gBatchFlag = FALSE;
	switch(rstOpenFile) {
	case 0: 
		//do nothing if error in open file 
		break;
	case 2:
		//initialize .trk file
		giSeqType = 1; //trk
		gbReplayFlag = TRUE;
		giClickCount=4;				
		img_ProcessLClick_Poly();
		
		dis_ShowImgCurrent();

		if (DEBUG==1){
			gbReplayFlag = FALSE;
		}

		//Functions[8].style = BS_PUSHBUTTON|WS_DISABLED; //disable log
		Functions[9].style = BS_PUSHBUTTON; //enable playRpt
		DrawButtons(hwndCtl, 0, TRUE);	
		
		break;
	case 3: 
		//batch tracking
		gBatchFlag = TRUE;			
		//based on .trk
		giSeqType = 1;
		//init batch parameters
		Batch_Init();
		break;
	default:
		//initialize image frame 
		giSeqType = 0; 		
		//Functions[8].style = BS_PUSHBUTTON; //enable log
		Functions[9].style = BS_PUSHBUTTON|WS_DISABLED; //disalbe playRpt
		DrawButtons(hwndCtl, 0, TRUE);	

		dis_ShowImgCurrent();
	}
	
	return TRUE;
}
/*****************************************************************************
 * Name:         AppExit                                                     *
 *****************************************************************************/
void AppExit(void)
{
	//release display buffer
	ReleaseBuffer();

	if(hpalette)
		DeleteObject(hpalette);

	//release imageio buffer
	iio_Exit();

	//release cv images
	cvReleaseImage(&gcImgCurrentHalf);
	cvReleaseImage(&gImgBitMap);
	cvReleaseImage(&gImgInitMaskHalf);
	cvReleaseImage(&gImgInitMask);

	//release tracker memory	
	switch(gTrackerIndex) {
	case 1:
		//tracker 1: HistogramShift - Enhanced meanshift by Foreground/Bkground		
		his_TrackCleanUp();
	case 2:
		//initial Meanshift Tracker
		msw_TrackCleanUp();					
		break;
	case 3:
		//template matching Tracker with Correlation
		temp_TrackCleanUp();
		break;
	case 4:		
	case 5:		
	case 6:
	case 7:
		//template matching Tracker with Correlation
		feat_TrackCleanUp();
		break;
	case 8:
//		graph_TrackCleanUp();
		break;
	case 9:
//		particle_TrackCleanUp();
		break;
	}
}
/*****************************************************************************
 * Name: AppIdle                                                             *
 *****************************************************************************/
BOOL AppIdle(void)
{
//	HDC hdc;
//	HDC hdc2;
	int waitFlag;

	// if this app is and icon, return TRUE and wait for a message
	if (IsIconic(hwndCtl))
		return TRUE;

	// if the user is leaning on the right mouse button let him have priority
	if (GetKeyState(VK_RBUTTON) >= 0)
	{
		//wait 
		//for (int i=1;i<100;i++){
			//wait();
		//}

		//get image
		waitFlag = GetImage();

		//print title		
		return waitFlag;
	}
	else
	{		
		return TRUE;      // background app; nothing to do.
	}	
}
/*****************************************************************************
 * Name: AppPaint                                                            *
 *****************************************************************************/
void AppPaint(HWND hwnd, HDC hdc)
{
	if (hpalette&&(PIXEL_TYPE==PBITS_Y8))
	{
		SelectPalette(hdc, hpalette, TRUE);
		RealizePalette(hdc);
	}

	SetDIBitsToDevice(
		hdc,
		0, ViewOffset, ImageMaxX, ImageMaxY,
		0, 0, 0, ImageMaxY,
		gpBits, (LPBITMAPINFO)&maphead, DIB_RGB_COLORS);
	
	
	if (gbClickInitFlag==FALSE) return;
	
	//draw rect
	if (gbTrackMood==TRUE){
		//draw obj box
		dis_DrawRect(hwndCtl, gRectCurrent);


		//display past tracked position
		if (DEBUG==1 || (gPosCount>2 && gbAutoPredictFlag==TRUE))
		{		
			dis_DrawTraceLine(hwndCtl, gPosPredict, gPosCount, PS_DASH, 2, RGB(255,0,0));			
		}
	}
	else{	
		dis_DrawRegion(hwnd, gRegionDisplay);
	}
}
/*****************************************************************************
 * Name: AppPaint2                                                            *
 *****************************************************************************/
void AppPaint2(HWND hwnd, HDC hdc)
{
	if (hpalette&&(PIXEL_TYPE==PBITS_Y8))
	//if (hpalette)
	{
		SelectPalette(hdc, hpalette, TRUE);
		RealizePalette(hdc);
	}

	SetDIBitsToDevice(
		hdc,
		0, ViewOffset, ImageMaxX, ImageMaxY,
		0, 0, 0, ImageMaxY,
		gpBits2, (LPBITMAPINFO)&maphead, DIB_RGB_COLORS);

	if (gTrackerIndex != 9) {//don't draw rect for particle filter
		//draw rect
		if (gbTrackMood==TRUE){
			//draw obj box
			dis_DrawRect(hwnd, gRectCurrentHalf);		
		}
		else{	
			if (gbReplayFlag==FALSE)
				dis_DrawRegion(hwnd, gRegionDisplayHalf);
		}
	}
	//draw klt bg lines
	if (gbKltFlag==TRUE && gDebugWinFlag == TRUE && gbClickInitFlag==TRUE && gbTrackMood==TRUE)
		dis_DrawKltBgLines();
}
/*****************************************************************************
 * Name:         CtlProc                                                     *
 * Description:  Main window procedure.                                      *
 *****************************************************************************/
LONG WINAPI CtlProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//MINMAXINFO *lpmmi;
	HDC hdc;
	PAINTSTRUCT ps;	
	int rstOpenFile;
	int clickX, clickY;
	
	switch (message)
	{
	case WM_CREATE:
		GetGlobals(hwnd, lParam);
		SetWindowPos(hwnd, NULL, 0, 0, WindowX, WindowY, SWP_NOMOVE | SWP_SHOWWINDOW | SWP_DRAWFRAME);
		return 0L;

	case WM_SIZE:	
		//size two windows together
		return 0L;

	case WM_PAINT:
		hdc = BeginPaint(hwnd,&ps);
		if (hwnd==hwndCtl)
			AppPaint(hwnd,hdc);
		else if (hwnd==hwndCtl2)
			AppPaint2(hwnd,hdc);
		EndPaint(hwnd,&ps);
		return 0L;

	case WM_TIMER:
		//if(fAcquire)
		{
			if(bTime == 0.0)
			{
				bTime = timeGetTime();
				FrameCount = 0;				
				//ErrorCount = 0;
				return 0L;
			}
			eTime = timeGetTime();
			dTime = eTime - bTime;
			iTime = dTime / CLOCKS_PER_SEC;
			if(iTime > 0)
				FPS = (int)fn_Round((double)FrameCount/iTime);				

			dis_PrintMessage();
			bTime = timeGetTime();
			FrameCount = 0;
		}
		return 0L;
	
	case WM_LBUTTONDOWN:
		if (hwnd == hwndCtl2) return 0L; //no response in debug window
		
		clickX = LOWORD(lParam);
		clickY = HIWORD(lParam);

		if (clickX>=ImageMaxX || clickY<ButtonHeight || clickY>=ImageMaxY+ButtonHeight) return 0L; //x,y out of bound
		
		//get the click point coordinate		
		gaClickPoint[giClickCount].x = LOWORD(lParam);		
		gaClickPoint[giClickCount].y = ImageMaxY-1+ButtonHeight-HIWORD(lParam); // flip the click point for bmp processing				

		giClickCount++;
		gbClickInitFlag=FALSE;
		gbTrackMood=FALSE;

		if (giClickCount<4) return 0L;
		
		//realease .trk mask and re-init
		iio_RealeaseMaskImage();		
		img_ProcessLClick_Poly();

		return 0L;

	case WM_RBUTTONDOWN:		
		return 0L;

	case WM_MOUSEMOVE:		
		if (hwnd!=hwndCtl || giClickCount==0 || giClickCount>3) 
			return 0L;
		
		clickX = LOWORD(lParam);
		clickY = HIWORD(lParam);
		if (clickX>=ImageMaxX || clickY<ButtonHeight || clickY>=ImageMaxY+ButtonHeight) return 0L; //x,y out of bound
	
		img_ProcessMouseMove(LOWORD(lParam), HIWORD(lParam));		
		
		return 0L;
		
	case WM_COMMAND:
		switch(wParam)
		{
		default:
			//DrawButtons(hwndCtl, 0, TRUE);
			break;

		case ID_READ:
			//no response in batch mood
			if (gBatchFlag==TRUE) return 0;

			giPlayFlag = 0;

			//open image file by prompt dialog
			rstOpenFile = iio_OpenFileByPrompt();
			if (rstOpenFile==0){ //canceled
				return 0;
			}
			if (rstOpenFile==3){			
				//batch tracking
				gBatchFlag = TRUE;
				Batch_Init();
				return 0;
			}

			//if no new frame, return
			if (gcImgCurrent==iio_GetInputImage() || iio_GetInputImage()==NULL){
				return 0L;
			}

			gbClickInitFlag=FALSE;
			gbTrackMood=FALSE;

			//update image			
			gcImgCurrent = iio_GetInputImage();
			ImageMaxX = iio_ImageMaxX();
			ImageMaxY = iio_ImageMaxY();
			giFrameInitial = iio_GetFrameNo();

			//HalfSize track image
			cvReleaseImage(&gcImgCurrentHalf);
			gcImgCurrentHalf = cvCreateImage(cvSize(ImageMaxX/2, ImageMaxY/2), 8, 3);
			if (gcImgCurrent!=NULL){			
				cvResize(gcImgCurrent, gcImgCurrentHalf, CV_INTER_LINEAR);
			}

			//mask image
			cvReleaseImage(&gImgInitMaskHalf);
			cvReleaseImage(&gImgInitMask);
			gImgInitMaskHalf = cvCreateImage(cvSize(ImageMaxX/2, ImageMaxY/2), 8, 1);
			gImgInitMask = cvCreateImage(cvSize(ImageMaxX, ImageMaxY), 8, 1);
			//bitmap image
			cvReleaseImage(&gImgBitMap);
			gImgBitMap = cvCreateImage(cvSize(ImageMaxX, ImageMaxY), 8, 1);

			//resize display window
			AppSetDisplay();
			
			//display current image
			dis_ShowImgCurrent();

			//reset predict
			gbPredictFlag = FALSE;			
			gPosCount = 0;

			//initilize track for different file type
			switch(rstOpenFile) {
			case 1:
				//initialize image frame file
				giSeqType =0;
				gbReplayFlag = FALSE;
				//Functions[8].style = BS_PUSHBUTTON; //enable Log
				Functions[9].style = BS_PUSHBUTTON|WS_DISABLED; //disalbe playRpt
				break;
			case 2:
				//create click region if opened a track file
				giSeqType = 1; //trk
				gbReplayFlag = TRUE;
				if (DEBUG==1) gbReplayFlag = FALSE;
				giClickCount=4;				
				img_ProcessLClick_Poly();
				//Functions[8].style = BS_PUSHBUTTON|WS_DISABLED; //disable log
				Functions[9].style = BS_PUSHBUTTON; //enable playRpt
				break;
			case 3:
				//batch tracking
				gBatchFlag = TRUE;
				//based on .trk
				giSeqType = 1;
				Batch_Init();
				break;
			}			
			DrawButtons(hwndCtl, 0, TRUE);

			return 0L;
			
		case ID_ALGORITHM:
			//no response in batch mood
			if (gBatchFlag==TRUE) return 0;

			giPlayFlag = 0;
			//prompt option dlg and select tracker
			dlg_Option();
			//if selected tracker is differenct from current tracker, rewind video and re-initialize with selected tracker
			if (gTrackerSelect!=gTrackerIndex || (gTrackerSelect==1 && gbBinChange==TRUE)){
				//release tracker memory	
				switch(gTrackerIndex) {
				case 1:
					//tracker 1: HistogramShift - Enhanced meanshift by Foreground/Bkground		
					his_TrackCleanUp();
				case 2:
					//initial Meanshift Tracker
					msw_TrackCleanUp();					
					break;
				case 3:
					//template matching Tracker with Correlation
					temp_TrackCleanUp();
					break;
				case 4:
				case 5:
				case 6:
				case 7:
					feat_TrackCleanUp();
					break;
				case 8:
//					graph_TrackCleanUp();
					break;
				case 9:
//					particle_TrackCleanUp();
					break;
				}
				gTkResult.FGImage = NULL;
				gTkResult.FGMask = NULL;
				gTkResult.ObjMask = NULL;

				gTrackerIndex=gTrackerSelect;
				
				if (gbClickInitFlag==TRUE){			
					img_RewindVideo();
					gbTrackMood=FALSE;					
				}
			}
			return 0L;

		case ID_BROWSEBACK:
			//no response in batch mood
			if (gBatchFlag==TRUE) return 0;

			//browse back
			giPlayFlag = 2; 

			//clear track box in tracking
			if (gbReplayFlag==FALSE){			
				gbClickInitFlag = FALSE;
				gbTrackMood=FALSE;
			}
			
			return 0L;

		case ID_TRACKFWD:
			//no response in batch mood
			if (gBatchFlag==TRUE) return 0;

			//track forward	
			giPlayFlag = 1; 
			gbTrackMood=TRUE;

			return 0L;

		case ID_STEPFWD:
			//no response in batch mood
			if (gBatchFlag==TRUE) return 0;

			giPlayFlag = 0;
			gbTrackMood=TRUE;
			
			//step to next frame	
			img_StepOneFrame(iio_GetFrameNo()+1);			

			return 0L;

		case ID_STOP:
			giPlayFlag = 0;					
			gbBatchStopFlag = TRUE;
			return 0L;

		case ID_REWIND:		
			//no response in batch mood
			if (gBatchFlag==TRUE) return 0;
			
			img_RewindVideo();
			gbTrackMood=FALSE;
			
			return 0L;

			
		case ID_KLT:
			//no response in batch mood
			if (gBatchFlag==TRUE) return 0;

			gbKltFlag = !gbKltFlag;	
			if (gbKltFlag==TRUE){
				Functions[10].style = BS_PUSHBUTTON; //enable predict
			}
			else{
				gbAutoPredictFlag = FALSE;
				Functions[10].style = BS_PUSHBUTTON|WS_DISABLED; //disalbe predict
			}
			DrawButtons(hwndCtl, 0, TRUE);

			return 0L;

		case ID_LOG:		
			//no response in batch mood
			if (gBatchFlag==TRUE) return 0;
			
			if (gbLogFlag==TRUE){	
				//turn off log flag
				gbLogFlag = FALSE;
				return 0L;
			}
			else{
				//turn on Log flag
				gbLogFlag = TRUE;

				//rewind video
				giPlayFlag = 0;
				gbTrackMood=FALSE;

				//reset track Rect
				gRectCurrent = gRectInit;			
				
				//rewind to intial frame
				img_StepOneFrame(giFrameInitial);								
				
				//initialize Histogram Tracker
				giClickCount = 4;
				img_ProcessLClick_Poly();

				//initial klt tracker
				klt_TrackInit(gcImgCurrentHalf);

				//reset predict
				gbPredictFlag = FALSE;
				gPosCount = 0;
			}

			return 0L;

		case ID_PLAYLOG:
			//no response in batch mood
			if (gBatchFlag==TRUE) return 0;

			if (giSeqType!=1){ //if not trk file
				return 0L;
			}
			//only work in trk sequence 

			//rewind to intial frame
			giPlayFlag = 0;	
			gbTrackMood=FALSE;
			img_StepOneFrame(giFrameInitial);				

			gbReplayFlag = !gbReplayFlag;
			if (gbReplayFlag==FALSE){				
				//enable Log button if Replay Log files is false
				//Functions[8].style = BS_PUSHBUTTON; //enable Log

				//create click region if opened a track file
				giClickCount=4;				
				img_ProcessLClick_Poly();
			}
			else{ 
				//Disable Log button if Replay Log files is true
				//Functions[8].style = BS_PUSHBUTTON|WS_DISABLED; //disalbe Log

				gbClickInitFlag = TRUE;
				img_RewindVideo();				
			}
			DrawButtons(hwndCtl, 0, TRUE);
			
			return 0L;
			
		case ID_PREDICT:	
			//no response in batch mood
			if (gBatchFlag==TRUE) return 0;
			
			gbAutoPredictFlag = !gbAutoPredictFlag;
			//Turn off predict mood if auto predict is off
			if (gbAutoPredictFlag==FALSE){			
				gbPredictFlag=FALSE;
			}
			return 0L;

		case ID_DEBUG:
			gDebugWinFlag = !gDebugWinFlag;
			if (gDebugWinFlag==TRUE)
				ShowWindow(hwndCtl2,SW_SHOWNORMAL);
			else 
				ShowWindow(hwndCtl2,SW_HIDE);
			return 0L;
		
		}
		break;

	case WM_DESTROY:
		KillTimer(hwnd, TEST_TIMER);
		PostQuitMessage(0);
		return 0L;
	}
	return(DefWindowProc(hwnd, message, wParam, lParam));
}

///////////////////////////////////////////////////////////////////////////////
BOOL dlg_LogDir()
//Promt user log dir dialog
{	
	//prompt log dir name
	if (DialogBox(hAppInst, MAKEINTRESOURCE(IDD_DIALOG_LOGDIR), hwndCtl, (DLGPROC)dlg_LogDirProc)==IDOK) 
    {
		//set logDir succeed
		return TRUE;
	}
    else 
    {
		//cancel log command
        return FALSE;
    }             
}

///////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK dlg_LogDirProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
//call back function for Log Directory dialog 
{
	char sLogPath[_MAX_PATH];
	//char sLogDirName[_MAX_DIR];
	int len;
	int i;

    switch (message) 
    { 
		case WM_INITDIALOG:
			
			//calculate log dir name
			io_CalcLogDirName(gLogDirName);
			//initial log dir
			strcpy(gLogDir, gExeLogPath);
			strcat(gLogDir, "\\");
			strcat(gLogDir, gLogDirName);			
			SetWindowText(GetDlgItem(hwndDlg, IDC_EDIT_LOGDIR), gLogDir); 
			SetFocus(GetDlgItem(hwndDlg, IDC_EDIT_LOGDIR)); 

			//init verions
			//initial RATIO 			
			switch(gLogVersion) {
			case 1:
				CheckRadioButton(hwndDlg, IDC_RADIO_LOGV1, IDC_RADIO_LOGV5, IDC_RADIO_LOGV1);
				break;
			case 2:
				CheckRadioButton(hwndDlg, IDC_RADIO_LOGV1, IDC_RADIO_LOGV5, IDC_RADIO_LOGV2);
				break;
			case 3:
				CheckRadioButton(hwndDlg, IDC_RADIO_LOGV1, IDC_RADIO_LOGV5, IDC_RADIO_LOGV3);
				break;
			case 4:
				CheckRadioButton(hwndDlg, IDC_RADIO_LOGV1, IDC_RADIO_LOGV5, IDC_RADIO_LOGV4);
				break;
			case 5:
				CheckRadioButton(hwndDlg, IDC_RADIO_LOGV1, IDC_RADIO_LOGV5, IDC_RADIO_LOGV5);
				break;
			}

			return FALSE;
		
        case WM_COMMAND: 
            switch (LOWORD(wParam)) 
            { 
                case IDOK: 
					//if edit is null or directory exist, reprompt
                    if (!GetDlgItemText(hwndDlg, IDC_EDIT_LOGDIR, sLogPath, MAX_PATH)) 
                    {     
						//reprompt if empty string						
						return FALSE;
					}
					else{
						//check if directory exist
						if (_chdir(sLogPath) == 0){ //directory exist							
							DialogBox(hAppInst, MAKEINTRESOURCE(IDD_DIALOG_EXIST), hwndDlg,(DLGPROC)dlg_DirExistProc);
							SetFocus(GetDlgItem(hwndDlg, IDC_EDIT_LOGDIR));
							return FALSE;
						}
						else{
							//Fall through and end dialog							
							EndDialog(hwndDlg, wParam); 
							//update global log directory							
							strcpy(gLogDir, sLogPath);
							//remember path without LogDirName
							len = strlen(sLogPath);
							//get reverse first "/" or "\"
							for (i=len-1;i>0;i--){				
								if (sLogPath[i]==92 || sLogPath[i]==47){ //equal "/" or "\"
									sLogPath[i] = '\0';
									break;
								}
							}
							strcpy(gExeLogPath, sLogPath);

							//GET Log version
							if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_LOGV1) == BST_CHECKED)
								gLogVersion=1;
							else if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_LOGV2) == BST_CHECKED){
								gLogVersion=2;
							}
							else if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_LOGV3) == BST_CHECKED){
								gLogVersion=3;
							}
							else if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_LOGV4) == BST_CHECKED){
								gLogVersion=4;
							}
							else{
								gLogVersion = 5;
							}
							return TRUE;
						}						
					}					
 
                case IDCANCEL: 
                    EndDialog(hwndDlg, wParam); 
                    return TRUE; 
            } 
    } 
    return FALSE; 
} 

///////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK dlg_DirExistProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
//call back function for Log Directory dialog 
{
    switch (message) 
    { 
        case WM_COMMAND: 
            switch (LOWORD(wParam)) 
            { 
                case IDOK: 
                    EndDialog(hwndDlg, wParam); 
                    return TRUE; 
            } 
    } 
    return FALSE; 
} 

///////////////////////////////////////////////////////////////////////////////
BOOL dlg_Option()
//Promt user option dialog
{	
	//prompt log dir name
	DialogBox(hAppInst, MAKEINTRESOURCE(IDD_DIALOG_OPTION), hwndCtl, (DLGPROC)dlg_OptionProc);

    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK dlg_OptionProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
//call back function for option dialog 
{		
    switch (message) 
    { 
		case WM_INITDIALOG:
			//initial RATIO 			
			switch(gTrackerIndex) {
			case 1:
				CheckRadioButton(hwndDlg, IDC_RADIO_TRACKER1, IDC_RADIO_TRACKER9, IDC_RADIO_TRACKER1);
				break;
			case 2:
				CheckRadioButton(hwndDlg, IDC_RADIO_TRACKER1, IDC_RADIO_TRACKER9, IDC_RADIO_TRACKER2);
				break;
			case 3:
				CheckRadioButton(hwndDlg, IDC_RADIO_TRACKER1, IDC_RADIO_TRACKER9, IDC_RADIO_TRACKER3);
				break;		
			case 4:
				CheckRadioButton(hwndDlg, IDC_RADIO_TRACKER1, IDC_RADIO_TRACKER9, IDC_RADIO_TRACKER4);
				break;		
			case 5:
				CheckRadioButton(hwndDlg, IDC_RADIO_TRACKER1, IDC_RADIO_TRACKER9, IDC_RADIO_TRACKER5);
				break;
			case 6:
				CheckRadioButton(hwndDlg, IDC_RADIO_TRACKER1, IDC_RADIO_TRACKER9, IDC_RADIO_TRACKER6);
				break;		
			case 7:
				CheckRadioButton(hwndDlg, IDC_RADIO_TRACKER1, IDC_RADIO_TRACKER9, IDC_RADIO_TRACKER7);
				break;		
			case 8:
				CheckRadioButton(hwndDlg, IDC_RADIO_TRACKER1, IDC_RADIO_TRACKER9, IDC_RADIO_TRACKER8);
				break;	
			case 9:
				CheckRadioButton(hwndDlg, IDC_RADIO_TRACKER1, IDC_RADIO_TRACKER9, IDC_RADIO_TRACKER9);
				break;
			}
			
			SetDlgItemInt(hwndDlg, IDC_EDIT_RBINS, gRbins, FALSE);
			SetDlgItemInt(hwndDlg, IDC_EDIT_GBINS, gGbins, FALSE);
			SetDlgItemInt(hwndDlg, IDC_EDIT_BBINS, gBbins, FALSE);
			
			if (gTrackerIndex==1){
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_RBINS),TRUE); 
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_GBINS),TRUE); 
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_BBINS),TRUE); 
			}
			else{
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_RBINS),FALSE); 
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_GBINS),FALSE); 
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_BBINS),FALSE); 
			}

			return FALSE;

        case WM_COMMAND: 			
            switch (LOWORD(wParam)) 
            { 
                case IDOK: 
					if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_TRACKER1) == BST_CHECKED){
						int rbin, gbin, bbin;
						
						gTrackerSelect=1;
						rbin = GetDlgItemInt(hwndDlg,IDC_EDIT_RBINS, NULL, FALSE);
						gbin = GetDlgItemInt(hwndDlg,IDC_EDIT_GBINS, NULL, FALSE);
						bbin = GetDlgItemInt(hwndDlg,IDC_EDIT_BBINS, NULL, FALSE);

						if (rbin!=gBbins || gbin!=gGbins || bbin!=gBbins){
							gbBinChange = TRUE;
							gRbins = rbin;
							gGbins = gbin;
							gBbins = bbin;
						}
						else{
							gbBinChange = FALSE;
						}
					}
					else if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_TRACKER2) == BST_CHECKED)
						gTrackerSelect=2;
					else if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_TRACKER3) == BST_CHECKED)
						gTrackerSelect=3;
					else if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_TRACKER4) == BST_CHECKED)
						gTrackerSelect=4;
					else if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_TRACKER5) == BST_CHECKED)
						gTrackerSelect=5;
					else if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_TRACKER6) == BST_CHECKED)
						gTrackerSelect=6;
					else if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_TRACKER7) == BST_CHECKED)
						gTrackerSelect=7;
					else if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_TRACKER8) == BST_CHECKED)
						gTrackerSelect=8;
					else if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_TRACKER9) == BST_CHECKED)
						gTrackerSelect=9;

                    EndDialog(hwndDlg, wParam); 
                    return TRUE; 
				case IDCANCEL: 
					gTrackerSelect = gTrackerIndex;
                    EndDialog(hwndDlg, wParam); 
                    return TRUE; 
				
				case  IDC_RADIO_TRACKER1: //click on tracker1
					EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_RBINS),TRUE); 
					EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_GBINS),TRUE); 
					EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_BBINS),TRUE); 
					return FALSE;

				default: //click on others				
					EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_RBINS),FALSE); 
					EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_GBINS),FALSE); 
					EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_BBINS),FALSE); 
					return FALSE;
            } 
    } 
    return FALSE; 
} 

/*****************************************************************************
 * Name: DrawButtons                                                         *
 *****************************************************************************/
void DrawButtons(HWND hwnd, LPARAM lParam, int draw)
{
	int i, rem;
	RECT Rect;

	GetClientRect(hwnd, (LPRECT)&Rect);
	ButtonWidth = Rect.right/NBUTTONS;
	rem = Rect.right % NBUTTONS;

	if(draw)
	{
		/*---------------------------------------------------------------------
		create the function buttons as child windows
		---------------------------------------------------------------------*/
		for(i=0; i<NBUTTONS; i++)
		{
			if(i == NBUTTONS-1)
				Functions[i].hwnd = CreateWindow("button", Functions[i].text,
				WS_CHILD | WS_VISIBLE | Functions[i].style,
				ButtonWidth*i, 0, ButtonWidth+rem, ButtonHeight,
				hwnd, (HMENU)Functions[i].id, hAppInst, NULL);
			else
				Functions[i].hwnd = CreateWindow("button", Functions[i].text,
				WS_CHILD | WS_VISIBLE | Functions[i].style,
				ButtonWidth*i, 0, ButtonWidth, ButtonHeight,
				hwnd, (HMENU)Functions[i].id, hAppInst, NULL);
		}
	}
	else
	{
		/*---------------------------------------------------------------------
		resize the function buttons
		---------------------------------------------------------------------*/
		for(i=0; i<NBUTTONS; i++)
		{
			if(i == NBUTTONS-1)
				SetWindowPos(Functions[i].hwnd,NULL,ButtonWidth*i,0,
				ButtonWidth+rem,ButtonHeight,SWP_SHOWWINDOW);
			else
				SetWindowPos(Functions[i].hwnd,NULL,ButtonWidth*i,0,
				ButtonWidth,ButtonHeight,SWP_SHOWWINDOW);
		}
	}
}
/*****************************************************************************
 * Name: DrawButtons2                                                         *
 *****************************************************************************/
void DrawButtons2(HWND hwnd, LPARAM lParam, int draw)
{
	int i, rem;
	RECT Rect;

	GetClientRect(hwnd, (LPRECT)&Rect);
	ButtonWidth = Rect.right/NBUTTONS;
	rem = Rect.right % NBUTTONS;

	if(draw)
	{
		/*---------------------------------------------------------------------
		create the function buttons as child windows
		---------------------------------------------------------------------*/
		for(i=0; i<NBUTTONS; i++)
		{
			if(i == NBUTTONS-1)
				Functions2[i].hwnd = CreateWindow("button", Functions[i].text,
				WS_CHILD | WS_VISIBLE | WS_DISABLED | Functions[i].style,
				ButtonWidth*i, 0, ButtonWidth+rem, ButtonHeight,
				hwnd, (HMENU)Functions[i].id, hAppInst, NULL);
			else
				Functions2[i].hwnd = CreateWindow("button", Functions[i].text,
				WS_CHILD | WS_VISIBLE | WS_DISABLED | Functions[i].style,
				ButtonWidth*i, 0, ButtonWidth, ButtonHeight,
				hwnd, (HMENU)Functions[i].id, hAppInst, NULL);
		}
	}
	else
	{
		/*---------------------------------------------------------------------
		resize the function buttons
		---------------------------------------------------------------------*/
		for(i=0; i<NBUTTONS; i++)
		{
			if(i == NBUTTONS-1)
				SetWindowPos(Functions2[i].hwnd,NULL,ButtonWidth*i,0,
				ButtonWidth+rem,ButtonHeight,SWP_SHOWWINDOW);
			else
				SetWindowPos(Functions2[i].hwnd,NULL,ButtonWidth*i,0,
				ButtonWidth,ButtonHeight,SWP_SHOWWINDOW);
		}
	}
}

/*****************************************************************************
 * Name: CreateGrayPalette                                                   *
 *****************************************************************************/
void CreateGrayPalette(void)
{
	int i;
	struct
	{
		WORD Version;
		WORD NumberOfEntries;
		PALETTEENTRY aEntries[256];
	} Palette;

	if (hpalette)
		DeleteObject(hpalette);

	Palette.Version = 0x300;
	Palette.NumberOfEntries = 256;
	for (i=0; i<256; ++i)
	{
		Palette.aEntries[i].peRed = i;
		Palette.aEntries[i].peGreen = i;
		Palette.aEntries[i].peBlue = i;
		Palette.aEntries[i].peFlags = PC_NOCOLLAPSE;
		maphead.colors[i].rgbBlue = i;
		maphead.colors[i].rgbGreen= i;
		maphead.colors[i].rgbRed = i;
		maphead.colors[i].rgbReserved = 0;
	}
	hpalette = CreatePalette((LOGPALETTE *)&Palette);
}
/*****************************************************************************/
int GetImage()
/*****************************************************************************/
{		
	int stopFlag;
	int stepRst;

	//if in batch mood, go to next until no more trk file or rpt file
	if (gBatchFlag==TRUE){
		if (gbBatchStopFlag==FALSE){		
			stepRst = img_StepOneFrame(iio_GetFrameNo()+1);
			gStepCount++;

			//default flag is not-stop
			stopFlag = 0;
			
			//if tracker stop (no more rpt file or failed)		
			if (stepRst==0 || gStepCount>=gBtkStepNum[gTrkIdx]){
				// if no more trk sequence, stop, else fetch next trk
				if (gTrkIdx>=gBtkFileCount-1){
					Batch_Exit(TRUE);
					stopFlag = 1;
				}
				else{
					gTrkIdx++;
					Batch_InitTrkFile(gBtkFileArray[gTrkIdx]);
					gStepCount=0;
					stopFlag = 0;
				}
			}
		}
		else{
			gBatchFlag = FALSE;
			Batch_Exit(TRUE);
			stopFlag = 1;
		}
		return stopFlag;
	}

	//if play mood, step one frame
	switch(giPlayFlag) {
	case 1: //track forward
		img_StepOneFrame(iio_GetFrameNo()+1);
		FrameCount++;		
		stopFlag = 0;
		break;
	case 2: //track backward
		img_StepOneFrame(iio_GetFrameNo()-1);
		FrameCount++;		
		stopFlag = 0;
		break;
	default:		
		//FrameCount = 0;		
		stopFlag = 1;
		break;
	}

	//FrameCount++;
	return stopFlag; 
}
/*****************************************************************************
 * Name: SetBitMapHead                                                       *
 *****************************************************************************/
void SetBitMapHead(void)
{
	// Set up bitmap header
	maphead.head.biSize=sizeof(BITMAPINFOHEADER);
	maphead.head.biWidth=ImageMaxX;
	maphead.head.biHeight=ImageMaxY;
	maphead.head.biPlanes=1;
	maphead.head.biBitCount=PIXEL_TYPE&0xFF;
	maphead.head.biCompression=BI_RGB;
	maphead.head.biSizeImage=0;
	maphead.head.biXPelsPerMeter=0;
	maphead.head.biYPelsPerMeter=0;
	maphead.head.biClrUsed=0;
	maphead.head.biClrImportant=0;
}
/*****************************************************************************
 * Name: RegisterWindowClasses                                               *
 *****************************************************************************/
void RegisterWindowClasses(HINSTANCE hInst, LPSTR szCmdLine, int sw)
{
	WNDCLASS  wc;
	static ATOM aClass = 0;

	memset(&wc,0,sizeof(wc));         /* clear stack variable */

	/* register the control window class */

	if(aClass == 0)
	{
		wc.style = CS_BYTEALIGNWINDOW | CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
		wc.lpfnWndProc   = CtlProc;
		wc.cbClsExtra    = 0;
		wc.cbWndExtra    = 0;
		wc.hInstance     = hAppInst;
		wc.hIcon         = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
		wc.hCursor       = LoadCursor (NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wc.lpszMenuName  = NULL;
		wc.lpszClassName = szAppName;

		RegisterClass (&wc);
	}
}
/*****************************************************************************
 * Name: CreateControlWindow                                                 *
 *****************************************************************************/
HWND CreateControlWindow(HINSTANCE hInst, LPSTR szCmdLine, int sw)
{
	HWND hwnd;

	hwnd = CreateWindow (szAppName,  // window class name
		szTitleBar,
		//(WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_THICKFRAME|WS_MINIMIZEBOX),
		(WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX),
		//WS_OVERLAPPEDWINDOW,         // window style
		CW_USEDEFAULT,               // initial x position
		CW_USEDEFAULT,               // initial y position
		CW_USEDEFAULT,               // initial x size
		CW_USEDEFAULT,               // initial y size
		NULL,                        // parent window handle
		NULL,                        // window menu handle
		hAppInst,                    // program instance handle
		NULL);                       // creation parameters

	return hwnd;
}
/*****************************************************************************
 * Name: GetGlobals                                                          *
 *****************************************************************************/
void GetGlobals(HWND hwnd, LPARAM lParam)
{
	int i;
	HDC hdc;
	TEXTMETRIC tm;                              /* text metric structure */

	/* get the maximum button length */
	ButtonTextLen = 0;
	for(i=0; i<NBUTTONS; i++)
		if(ButtonTextLen < strlen(Functions[i].text))
			ButtonTextLen = strlen(Functions[i].text);

	hdc = GetDC (hwndCtl);
	SelectObject(hdc, GetStockObject(SYSTEM_FONT));
	GetTextMetrics (hdc, &tm);
	ReleaseDC (hwndCtl, hdc);
	cxChar = tm.tmAveCharWidth;
	cyChar = tm.tmHeight + tm.tmExternalLeading;
	ButtonHeight = 4*cyChar/3;
	ViewOffset = ButtonHeight;

	/* calculate the initial window size */
	WindowX = max(ImageMaxX,500) + (GetSystemMetrics(SM_CXFRAME)*2);
	WindowY = max(ImageMaxY,240) + ButtonHeight + GetSystemMetrics (SM_CYCAPTION) + GetSystemMetrics (SM_CYFRAME)*2;
}
/*****************************************************************************
 * Name: AllocBuffer                                                         *
 *****************************************************************************/
int AllocBuffer(void)
{
	hBuf = GlobalAlloc(GMEM_FIXED, (long)ImageMaxX * ImageMaxY*(((PIXEL_TYPE&0xFF)+7L)>>3));
	if(hBuf == NULL)
	{
		MessageBox(0, "Cannot allocate buffer", szAppName, MB_OK);
		return FALSE;
	}
	gpBits = (BYTE __PX_HUGE *)GlobalLock(hBuf);

	hBuf2 = GlobalAlloc(GMEM_FIXED, (long)ImageMaxX * ImageMaxY*(((PIXEL_TYPE&0xFF)+7L)>>3));
	//hBuf2 = GlobalAlloc(GMEM_FIXED, (long)ImageMaxX * ImageMaxY);
	if(hBuf2 == NULL)
	{
		MessageBox(0, "Cannot allocate buffer", szAppName, MB_OK);
		return FALSE;
	}
	gpBits2 = (BYTE __PX_HUGE *)GlobalLock(hBuf2);
	memset(gpBits2, 0, ImageMaxX*ImageMaxY*3); //set zero

	gImgDisplayMain = cvCreateImageHeader(cvSize(ImageMaxX, ImageMaxY), 8, 3);
	gImgDisplayMain->imageData = gpBits;
	gImgDisplayMain->imageDataOrigin = gpBits;

	gImgDisplayDebug = cvCreateImageHeader(cvSize(ImageMaxX, ImageMaxY), 8, 3);
	gImgDisplayDebug->imageData = gpBits2;
	gImgDisplayDebug->imageDataOrigin = gpBits2;

	return TRUE;
}
/*****************************************************************************/
void ReleaseBuffer()
/*****************************************************************************/
{
	cvReleaseImageHeader(&gImgDisplayMain);
	cvReleaseImageHeader(&gImgDisplayDebug);

	if(hBuf)
	{
		GlobalUnlock(hBuf);
		GlobalFree(hBuf);
	}
	if(hBuf2)
	{
		GlobalUnlock(hBuf2);
		GlobalFree(hBuf2);
	}
}

/*****************************************************************************
 * Name: HourGlass                                                           *
 *****************************************************************************/
void HourGlass(int on)
{
	static HCURSOR hCursor;

	if(on == ON)
	{
		hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
		ShowCursor(TRUE);
	}
	else
	{
		ShowCursor(FALSE);
		SetCursor(hCursor);
	}
}

/******************************************************************************/
void dis_PrintMessage(){		
	char	*frameName ;	
	char	szTitleBar2[100];
	char	trackerDesc[5];
	char	*kltDesc;
	char	*logDesc;
	char	*predictDesc;
	char	sTkscore[15];
	char	sAffineMtx[6][20];
	char	sZoomAccum[10];
	int		i;

	//if (FPS>30) FPS = 30;
	
	if (gbReplayFlag==TRUE){
		strcpy(trackerDesc,"On");
		frameName = iio_GetFileTitle();
	}
	else{
		strcpy(trackerDesc,"Off");
		frameName = iio_GetImageTitle();
	}

	if (gbKltFlag==TRUE)	kltDesc = "on";
	else					kltDesc = "off";

	if (gbLogFlag==TRUE)	logDesc = "on";
	else					logDesc = "off";

	if (gbAutoPredictFlag==TRUE)	predictDesc = "on";
	else					predictDesc = "off";
	
	fn_DoubleToStr2(gTkResult.score,2, sTkscore);

	//set title 1
	if (gBatchFlag==TRUE){
		wsprintf(szTitleBar,"%s; Batch mode, press STOP to quit ",frameName);
	}
	else{	
		wsprintf(szTitleBar,"%s; Klt:%s; Log:%s; Rpt:%s; Predict:%s; ",frameName, kltDesc, logDesc, trackerDesc, predictDesc);
	}
	SetWindowText(hwndCtl, szTitleBar);

	//set title 2
	for(i=0;i<6;i++){
		fn_DoubleToStr2(gAffineMatrix[i], 2, sAffineMtx[i]);		
	}
	fn_DoubleToStr2(gZoomAccum, 2, sZoomAccum);		
	wsprintf(szTitleBar2, "FPS:%d; Zoom:%s; score:%s; %s",FPS, sZoomAccum, sTkscore, feat_GetFeatLabel());
	SetWindowText(hwndCtl2, szTitleBar2);


	return;
}

///////////////////////////////////////////////////////////////////////////////
int img_StepOneFrame(int nextFrameNo)
//Step to next frame in sequence
{				
	IplImage *imgForeground;
	int		i;
	float	zoom;
	int		predictReady;
	clock_t tstart, tend;
	//if in the Replay Mood, read trk/rpt file and display it
	if (gbReplayFlag == TRUE){
		if (nextFrameNo == giFrameInitial){
			iio_ReplayTrkFile();	
			
			//get image
			gcImgCurrent = iio_GetInputImage();			

			//reset track Rect
			gRectCurrent = gRectInit;

			//stop backward browsing
			giPlayFlag = 0;

			//display changes
			dis_ShowImgCurrent();
			return 2;
		}
		
		//otherwise open rpt file
		iio_ReplayRptFile(nextFrameNo);	

		//if no new file, stop looping
		if (iio_GetFrameNo() !=nextFrameNo){
			giPlayFlag = 0;
		}
		gcImgCurrent = iio_GetInputImage();
		gRectCurrent.left = gaClickPoint[0].x;
		gRectCurrent.top = gaClickPoint[0].y;
		gRectCurrent.right = gaClickPoint[2].x;
		gRectCurrent.bottom = gaClickPoint[2].y;

		dis_ShowImgCurrent();
		return 2;
	}

	//Read next frame from file
	iio_ReadFileByIndex(nextFrameNo);

	//if no new frame, return
	if (gcImgCurrent==iio_GetInputImage() || iio_GetInputImage()==NULL){	
		giPlayFlag=0;
		return 0;
	}
	
	//update image sequence info.	
	gcImgCurrent = iio_GetInputImage();
	cvResize(gcImgCurrent, gcImgCurrentHalf, CV_INTER_LINEAR);
	
	//if not in initialized, return
	if (gbClickInitFlag==FALSE || gbTrackMood==FALSE){		
		//display current image and rect
		dis_ShowImgCurrent();
		return 0;
	}
	
	//klt tracker
	if (gbKltFlag==TRUE){
		klt_TrackNextFrame(gcImgCurrentHalf, gAffineMatrix);

		//get zoom scale factor from affine matrix		
		zoom = (float)sqrt(fn_Abs(gAffineMatrix[0]*gAffineMatrix[4] - gAffineMatrix[1]*gAffineMatrix[3]));
		
		//if zoom scale is not 1 (different above threshold), accumulate zoom change.
		if (fabs(zoom-1)>0.002){		
			gZoomAccum = gZoomAccum*zoom;
		}

		//change size if not in predict mood
		if (gbPredictFlag==FALSE)
		{		
			//change rect size based on scale factor accumulation
			gBlobWidth = (int)fn_Round(gBlobWidthInit * gZoomAccum);
			gBlobHeight = (int)fn_Round(gBlobHeightInit * gZoomAccum);

			//adjust the current Blob size
			utl_RectSizeAdjust(&gRectCurrentHalf, gBlobWidth, gBlobHeight, ImageMaxX/2, ImageMaxY/2);
		}
	}
	else{		
		for (i=0;i<6;i++){		
			gAffineMatrix[i] = 0;
		}
	}

	//runing tracker on this frame
	switch(gTrackerIndex) {
	case 1: 
		//HistogramShift - Enhanced meanshift by Foreground/Bkground		
		his_TrackNextFrame(gcImgCurrentHalf, gRectCurrentHalf, &gTkResult);
		break;
	case 2:
		//Meanshift tracker
		msw_TrackNextFrame(gcImgCurrentHalf, gRectCurrentHalf, &gTkResult);		
		break;
	case 3:
		//template matching tracker with Correlation
		temp_TrackNextFrame_Coeff(gcImgCurrentHalf, gRectCurrentHalf, &gTkResult);		
		break;
	case 4:  
		//feature shift
		feat_TrackNextFrame(gcImgCurrentHalf, gRectCurrentHalf, &gTkResult);
		break;
	case 5:  
		//feature shift adaptive
		feat_TrackNextFrame_Adapt(gcImgCurrentHalf, gRectCurrentHalf, &gTkResult);
		break;
	case 6:  
		//Peak Diff feature shift adaptive
		feat_TrackNextFrame_PeakDiff(gcImgCurrentHalf, gRectCurrentHalf, &gTkResult);
		break;
	case 7:  
		//Peak Diff feature shift adaptive
		feat_TrackNextFrame_PeakDiff_Adapt(gcImgCurrentHalf, gRectCurrentHalf, &gTkResult);
		break;
	case 8:  
		//GraphCut Tracker
//		graph_TrackNextFrame(gcImgCurrentHalf, gRectCurrentHalf, &gTkResult);
		break;
	case 9:  
		//particle filter tracker
//		particle_TrackNextFrame(gcImgCurrent, gRectCurrent, &gTkResult);		 
		break;	
	//default:
	}
	
	gRectCurrentHalf = gTkResult.targetBox;
	utl_RectCheckBound(&gRectCurrentHalf, ImageMaxX/2, ImageMaxY/2);

	//get foreground image/bitmap for report purpose	
	//imgForeground = gTkResult.FGImage;
	imgForeground = gTkResult.FGMask;
	cvSetZero(gImgBitMap);
	//cvResize(imgForeground, gImgBitMap, CV_INTER_LINEAR);
	if (imgForeground != NULL){
		cvResize(imgForeground, gImgBitMap, CV_INTER_NN);
	}
	
	//if occluded, predict motion
	gTrackCount++;
	if (gbAutoPredictFlag==TRUE){
		if (gbPredictFlag==FALSE && gTkResult.occlusion==TRUE && gTrackCount>=3){
			gTrackCount = 0;
			gbPredictFlag=TRUE;
		}
		else if(gbPredictFlag==TRUE && gTkResult.occlusion==FALSE && gTrackCount>=3){
			gTrackCount = 0;
			gbPredictFlag=FALSE;
		}

	}	

	//predict motion on current frame
	if(gbKltFlag==TRUE){	
		//update the Blob Position array to current camera coordinate, and shift one position in array
		for (i=POSNUM-1;i>0;i--){
			//affine transform
			gPosPredict[i].x = 2*klt_CalcAffineX(gPosPredict[i-1].x/2, gPosPredict[i-1].y/2);
			gPosPredict[i].y = 2*klt_CalcAffineY(gPosPredict[i-1].x/2, gPosPredict[i-1].y/2);
			utl_PixelCheckBound_Float(&gPosPredict[i].x, &gPosPredict[i].y, ImageMaxX, ImageMaxY);
		}
		
		//get effective position count
		gPosCount = min(gPosCount++, POSNUM);

		//predict the blob position and get gAvgDx, gAvgDy;
		predictReady = img_PredictMotion();
	}
	
	//shift the blob based on prediction if flag is on	
	if (gbPredictFlag==TRUE && predictReady==1)
	{	
		//Since recent frames are occluded, rePredict position in gPosPredict[0]-[4] based on 5 frames earlier		
		for(i=gIgnoreFrameNum-1;i>=0;i--){		
			gPosPredict[i].x = gPosPredict[gIgnoreFrameNum].x+ gAvgDx*(gIgnoreFrameNum-i);
			gPosPredict[i].y = gPosPredict[gIgnoreFrameNum].y+ gAvgDy*(gIgnoreFrameNum-i);
			utl_PixelCheckBound_Float(&gPosPredict[i].x, &gPosPredict[i].y, ImageMaxX, ImageMaxY);
		}
		
		//reset target rect based on prediction
		gRectCurrentHalf.left	= (long)fn_Round(gPosPredict[0].x/2 - gBlobWidth/2);
		gRectCurrentHalf.right	= (long)fn_Round(gPosPredict[0].x/2 + gBlobWidth/2);
		gRectCurrentHalf.top	= (long)fn_Round(gPosPredict[0].y/2 - gBlobHeight/2);
		gRectCurrentHalf.bottom	= (long)fn_Round(gPosPredict[0].y/2 + gBlobHeight/2);	
		utl_RectCheckBound(&gRectCurrentHalf, ImageMaxX/2, ImageMaxY/2);
	}	
	else{
		//merge predict position with actual tracked position 
		gPosPredict[0].x = (float)gRectCurrentHalf.left+gRectCurrentHalf.right;
		gPosPredict[0].y = (float)gRectCurrentHalf.top+gRectCurrentHalf.bottom;		
	}
	
	//update full size RECT for display and Log report purpose
	gRectCurrent.left	= gRectCurrentHalf.left*2;
	gRectCurrent.top	= gRectCurrentHalf.top*2;
	gRectCurrent.right	= gRectCurrentHalf.right*2;
	gRectCurrent.bottom	= gRectCurrentHalf.bottom*2;
	
	
	//print track report
	if (gbLogFlag == TRUE){		
		tReport.time		= iio_GetFrameNo();
		tReport.framePath	= iio_GetFilePath();
		tReport.boxX		= gRectCurrent.left;
		tReport.boxY		= ImageMaxY-1 - gRectCurrent.bottom;
		tReport.boxWid		= abs(gRectCurrent.right - gRectCurrent.left+1);
		tReport.boxHgt		= abs(gRectCurrent.bottom - gRectCurrent.top+1);		
		tReport.imgWidth	= ImageMaxX;
		tReport.imgHeight	= ImageMaxY;
		switch(gLogVersion) {
		case 1:
			//CMU version 1
			io_PrintReport();
			break;		
		case 2:
			//CID version 0.9
			io_PrintReport_CID09();				
			break;
		case 3:
			//CID version 1.0
			io_PrintReport_CID10();
			break;
		case 4:
			//CMU version 2.0
			io_PrintReport_V20();
			break;
		case 5:
			//CMU version 2.1
			io_PrintReport_V21();
			break;
		} 
			
		
	}		

	//flip the points for draw purpose on screen window
	for (i=0;i<gPosCount;i++){		
		gPosDraw[i].x = (long)gPosPredict[i].x;
		gPosDraw[i].y = ImageMaxY-1 + ButtonHeight - (long)gPosPredict[i].y;
	}
	
	//display current image and rect
	dis_ShowImgCurrent();

	return 1;
}

///////////////////////////////////////////////////////////////////////////////
void AppSetDisplay()
//set display window and related parameters based on ImageMaxX/ImageMaxY
{
	int left;
	int top;

	int screenx = GetSystemMetrics(SM_CXSCREEN);
	int screeny = GetSystemMetrics(SM_CYSCREEN);

	ReleaseBuffer();
	SetBitMapHead();
	AllocBuffer();
	GetGlobals(hwndCtl,0);

	MoveWindow(hwndCtl,0,0,WindowX,WindowY,1);
	//MoveWindow(hwndCtl,screenx-WindowX,screeny-35-WindowY,WindowX,WindowY,1);	
	InvalidateRect(hwndCtl, NULL, TRUE);
	DrawButtons(hwndCtl, 0, FALSE); //resize buttons
	
	//debug windows 
	left = min(WindowX,screenx-WindowX);
	top = (left>=WindowX-20?0:min(WindowY, screeny-30-WindowY));		
	
	left = max(left,0);
	top = max(top,0);
	MoveWindow(hwndCtl2,left,top,WindowX,WindowY,TRUE);
	//MoveWindow(hwndCtl2,0,0,WindowX,WindowY,TRUE);
	InvalidateRect(hwndCtl2, NULL, TRUE);
	DrawButtons2(hwndCtl2, 0, FALSE); //resize buttons

	SetActiveWindow(hwndCtl);
}

///////////////////////////////////////////////////////////////////////////////
void dis_ShowImgCurrent()
//copy current image into main window buffer
{	
	//if batch mood, return
	//if (gBatchFlag==TRUE) return;

	//if no image, return
	if (gcImgCurrent==NULL) return;

	cvCopy(gcImgCurrent, gImgDisplayMain, NULL);

	//paint image	
	AppPaint(hwndCtl, GetDC(hwndCtl));
	AppPaint2(hwndCtl2, GetDC(hwndCtl2));

	//display debug image
	dis_ShowImgDebug();

	//print title
	dis_PrintMessage();
}

///////////////////////////////////////////////////////////////////////////////
void dis_ShowImgDebug()
//copy current image into main window buffer
{
	CvRect roi;	
	IplImage* imgDisplay;

	//if replay, no debug display
	if (gbReplayFlag==TRUE) return;

	memset(gpBits2, 0, ImageMaxX*ImageMaxY*3);

	//display particles for particle filter
	if (gTrackerIndex==9) {
//		imgDisplay = particle_GetParticleImg();
		if (imgDisplay!=NULL){
			cvCopy(imgDisplay, gImgDisplayDebug, NULL);
		}	
		return;
	}

	//display half image
	if (gbClickInitFlag==TRUE && gcImgCurrentHalf!=NULL){
		roi.x = 0;
		roi.y = ImageMaxY/2;
		roi.width = gcImgCurrentHalf->width;
		roi.height = gcImgCurrentHalf->height;
		cvSetImageROI(gImgDisplayDebug, roi);
		cvCopy(gcImgCurrentHalf, gImgDisplayDebug, NULL);
		cvResetImageROI(gImgDisplayDebug);
	}
	
	//display klt features image
	if (gbKltFlag==TRUE)		
	{
		IplImage *imgFeature = klt_GetFeatureImage();
		if (gbClickInitFlag==TRUE && gbTrackMood==TRUE && imgFeature!=NULL){
			roi.x = ImageMaxX/2;
			roi.y = ImageMaxY/2;
			roi.width = gcImgCurrentHalf->width;
			roi.height = gcImgCurrentHalf->height;
			cvSetImageROI(gImgDisplayDebug, roi);
			cvCopy(imgFeature, gImgDisplayDebug, NULL);
			cvResetImageROI(gImgDisplayDebug);
		}
	}

	//display foreground image
	{		
		IplImage *imgForeground = gTkResult.FGImage;
		//if (gbClickInitFlag==TRUE && gbTrackMood==TRUE && imgForeground!=NULL){
		if (gbClickInitFlag==TRUE && imgForeground!=NULL){
			roi.x = 0;
			roi.y = 0;
			roi.width = gcImgCurrentHalf->width;
			roi.height = gcImgCurrentHalf->height;
			cvSetImageROI(gImgDisplayDebug, roi);
			cvSetImageCOI(gImgDisplayDebug, 2);
			cvCopy(imgForeground, gImgDisplayDebug, NULL);
			cvSetImageCOI(gImgDisplayDebug, 0);	
			cvResetImageROI(gImgDisplayDebug);
		}
	}

	//display obj mask image
	{		
		//IplImage *imgObjMask = gTkResult.ObjMask;//objMask is 3 channels
		IplImage *imgObjMask = gTkResult.FGMask; //Fgmask is 1 channel 

		//if (gbClickInitFlag==TRUE && gbTrackMood==TRUE && imgForeground!=NULL){
		if (gbClickInitFlag==TRUE && imgObjMask!=NULL){
			roi.x = ImageMaxX/2;
			roi.y = 0;
			roi.width = gcImgCurrentHalf->width;
			roi.height = gcImgCurrentHalf->height;
			cvSetImageROI(gImgDisplayDebug, roi);
			//cvCopy(imgObjMask, gImgDisplayDebug, NULL); 			
			cvSetImageCOI(gImgDisplayDebug, 1);
			cvCopy(imgObjMask, gImgDisplayDebug, NULL); 
			cvSetImageCOI(gImgDisplayDebug, 2);
			cvCopy(imgObjMask, gImgDisplayDebug, NULL); 
			cvSetImageCOI(gImgDisplayDebug, 3);
			cvCopy(imgObjMask, gImgDisplayDebug, NULL); 
			cvSetImageCOI(gImgDisplayDebug, 0);	
			cvResetImageROI(gImgDisplayDebug);
		}
	}	
	//paint image		
	AppPaint2(hwndCtl2, GetDC(hwndCtl2));
}

///////////////////////////////////////////////////////////////////////////////
void dis_DrawRect(HWND hwnd, RECT rect)
//display current rect in main window
{
	HDC hdc;
//	LOGBRUSH lb; 
    HPEN hPen, hPenOld; 

	//display rect track window
	if (gbClickInitFlag==FALSE) return;

    hdc = GetDC(hwnd); 

//	// Initialize the pen's brush.
//	lb.lbStyle = BS_SOLID; 
//	lb.lbColor = RGB(255,0,0); 
//	lb.lbHatch = 0; 
	
	// define new pen
	//hPen = ExtCreatePen(PS_COSMETIC | PS_SOLID, 1, &lb, 0, NULL); 
	hPen = CreatePen(PS_SOLID, giPenWidth, crPenColor);
	hPenOld = SelectObject(hdc, hPen); 

	//flip back rect for display purpose
	rect.top = ImageMaxY + ButtonHeight - rect.top;
	rect.bottom = ImageMaxY + ButtonHeight - rect.bottom;
	
	//if the rect buttom is in Button Area, ignore it.
	if(rect.bottom<=ButtonHeight) rect.bottom = ButtonHeight+1;

	//draw rect
	MoveToEx(hdc, rect.left, rect.top, NULL); 
	LineTo(hdc, rect.right, rect.top); 
	LineTo(hdc, rect.right, rect.bottom); 
	LineTo(hdc, rect.left, rect.bottom); 
	LineTo(hdc, rect.left, rect.top); 

	//recover old pen
	SelectObject(hdc, hPenOld); 
	DeleteObject(hPen); 
    
	//release Device Context
	ReleaseDC(hwnd, hdc); 
}

///////////////////////////////////////////////////////////////////////////////
void dis_DrawTraceLine(HWND hwnd, FPOINT* pts, int ptCount, int penStyle, int penWidth, COLORREF penColor)
//display current rect in main window
{
	HDC hdc;
//	LOGBRUSH lb; 
    HPEN hPen, hPenOld; 
	int i;
	POINT predictPt;
	int frameNum=(int)(gEffectNum*0.6); //predict ahead number of frames;
	
	//display rect track window
	if (gbClickInitFlag==FALSE) return;

    hdc = GetDC(hwnd); 

	// define new pen
	//hPen = ExtCreatePen(PS_COSMETIC | PS_SOLID, 1, &lb, 0, NULL); 
	hPen = CreatePen(penStyle, penWidth, penColor);
	hPenOld = SelectObject(hdc, hPen); 

	//draw history line in red
	MoveToEx(hdc, gPosDraw[0].x, gPosDraw[0].y, NULL); 
	for (i=1;i<ptCount;i++){
		LineTo(hdc, gPosDraw[i].x, gPosDraw[i].y); 
	}
	
	if (gbPredictFlag==TRUE)
	{
		//display predict mood
		RECT promptArea;

		promptArea.left = 30;
		promptArea.top = ButtonHeight+20;
		promptArea.right = promptArea.left + 100;
		promptArea.bottom = promptArea.top + 100;

		DrawText(hdc, "Predict Motion", 14, &promptArea, DT_LEFT);
	

		//predict line	in green
		hPen = CreatePen(penStyle, penWidth, RGB(0,255,0));
		SelectObject(hdc, hPen);	
		MoveToEx(hdc, gPosDraw[0].x, gPosDraw[0].y, NULL); 
		for(i=1;i<frameNum;i++){
			predictPt.x = gPosDraw[0].x + (int)fn_Round(i*gAvgDx);
			predictPt.y = gPosDraw[0].y - (int)fn_Round(i*gAvgDy);
			
			//draw the point if it is not in button area
			if(predictPt.y>ButtonHeight&&predictPt.x<ImageMaxX)
				LineTo(hdc, predictPt.x, predictPt.y);
		}
	}
	//recover old pen
	SelectObject(hdc, hPenOld); 
	DeleteObject(hPen); 
    
	//release Device Context
	ReleaseDC(hwnd, hdc); 
}

///////////////////////////////////////////////////////////////////////////////
void dis_DrawKltBgLines()
//display current rect in main window
{
	HDC hdc;
    HPEN hPen, hPenOld; 
	int i;
	
	if (ftBg == NULL) return;
	
    hdc = GetDC(hwndCtl2); 

	// define new pen
	hPen = CreatePen(PS_SOLID, 2, RGB(255,0,0));
	hPenOld = SelectObject(hdc, hPen); 

	//draw history line in red	
	for (i=0;i<ftBg->nFeatures;i++){
		//flip the points for draw purpose on screen window
		MoveToEx(hdc, ImageMaxX/2+(int)ftBg->feature[i][0]->x,ButtonHeight+ImageMaxY/2-1-(int)ftBg->feature[i][0]->y, NULL); 
		LineTo(hdc, ImageMaxX/2+(int)ftBg->feature[i][1]->x, ButtonHeight+ImageMaxY/2-1-(int)ftBg->feature[i][1]->y); 
	}
	
	//recover old pen
	SelectObject(hdc, hPenOld); 
	DeleteObject(hPen); 
    
	//release Device Context
	ReleaseDC(hwndCtl2, hdc); 
}

///////////////////////////////////////////////////////////////////////////////
void img_ProcessLClick_Poly()
//Process after click
{
	int i;
	IplImage *trkMaskImage;

	//gbClickInitFlag=FALSE;
	//gbTrackMood=FALSE;

	//if (giClickCount<4){
	//	return;
	//}			
	
	//calculate other points
	for(i=0;i<4;i++){	
		gaClickPointDisplay[i].x = gaClickPoint[i].x;
		gaClickPointDisplay[i].y = ImageMaxY+ButtonHeight-gaClickPoint[i].y; 
		gaClickPointHalf[i].x = gaClickPoint[i].x/2;
		gaClickPointHalf[i].y = gaClickPoint[i].y/2;
		gaClickPointHalfDisplay[i].x = gaClickPointHalf[i].x;
		gaClickPointHalfDisplay[i].y = ImageMaxY+ButtonHeight-gaClickPointHalf[i].y;
	}

	//initilize/update rec region
	gRegionCurrent = CreatePolygonRgn(gaClickPoint, giClickCount, WINDING);
	gRegionCurrentHalf = CreatePolygonRgn(gaClickPointHalf, giClickCount, WINDING);
	gRegionDisplay = CreatePolygonRgn(gaClickPointDisplay, giClickCount, WINDING);
	gRegionDisplayHalf = CreatePolygonRgn(gaClickPointHalfDisplay, giClickCount, WINDING);

	//reInit frameNo
	giFrameInitial = iio_GetFrameNo();	

	//reset count
	giClickCount=0;

	//initialize tracker
	gbClickInitFlag=TRUE;	

	GetRgnBox(gRegionCurrent, &gRectInit);
	
	gRectCurrent.left	= gRectInit.left;
	gRectCurrent.top	= gRectInit.top;
	gRectCurrent.right	= gRectInit.right;
	gRectCurrent.bottom	= gRectInit.bottom;
	
	//tracking on half image
	gRectCurrentHalf.left	= gRectCurrent.left/2;
	gRectCurrentHalf.top	= gRectCurrent.top/2;
	gRectCurrentHalf.right	= gRectCurrent.right/2;
	gRectCurrentHalf.bottom	= gRectCurrent.bottom/2;		
	gBlobWidthInit	= gRectCurrentHalf.right-gRectCurrentHalf.left;
	gBlobHeightInit = gRectCurrentHalf.bottom - gRectCurrentHalf.top;
	gBlobWidth = gBlobWidthInit;
	gBlobHeight = gBlobHeightInit;

	//create region mask image, 1 within region and 0 outside region.
	cvSetZero(gImgInitMask);
	trkMaskImage = iio_GetMaskImage();
	if (trkMaskImage==NULL)
	{
		img_CreateRegionMask(gRegionCurrentHalf, gImgInitMaskHalf);
		img_CreateRegionMask(gRegionCurrent, gImgInitMask);
	}
	else{
		cvResize(trkMaskImage, gImgInitMaskHalf, CV_INTER_NN);	
		cvCopy(trkMaskImage, gImgInitMask, NULL);
	}
	//cvSaveImage("c:\\CTracker\\gImgInitMaskHalf.bmp", gImgInitMaskHalf);
	//cvSaveImage("c:\\CTracker\\gImgInitMask.bmp", gImgInitMask);

	//Inititilize tracker
	switch(gTrackerIndex) {
	case 1:
		//tracker 1: HistogramShift - Enhanced meanshift by Foreground/Bkground		
		his_TrackInit_Bins(gcImgCurrentHalf, gImgInitMaskHalf, gRectCurrentHalf, gRbins, gGbins, gBbins);
		//gTkResult.FGImage = NULL;
		gTkResult.FGImage = his_GetBackProjImage();
		break;

	case 2:
		//initial Meanshift Tracker
		msw_TrackInit(gcImgCurrentHalf,gImgInitMaskHalf, gRectCurrentHalf);
		//msw_PrintHistogram("result/modelhist_rect.txt");
		gTkResult.FGImage = NULL;
		break;
	case 3:
		//template matching Tracker with Correlation
		temp_TrackInit_Coeff(gcImgCurrentHalf, gImgInitMaskHalf, gRectCurrentHalf);
		gTkResult.FGImage = NULL;
		break;

	case 4: 
		//feature shift
		feat_TrackInit(gcImgCurrentHalf, gImgInitMaskHalf, gRectCurrentHalf);
		gTkResult.FGImage = feat_GetWeightImage();
		break;

	case 5: 
		//feature shift adaptive
		feat_TrackInit(gcImgCurrentHalf, gImgInitMaskHalf, gRectCurrentHalf);
		gTkResult.FGImage = feat_GetWeightImage();
		break;

	case 6: 
		//Peak Diff feature shift
		feat_TrackInit_PeakDiff(gcImgCurrentHalf, gImgInitMaskHalf, gRectCurrentHalf);
		gTkResult.FGImage = feat_GetWeightImage();
		break;

	case 7:
		//Peak Diff feature shift adaptive
		feat_TrackInit_PeakDiff(gcImgCurrentHalf, gImgInitMaskHalf, gRectCurrentHalf);
		gTkResult.FGImage = feat_GetWeightImage();
		break;
	
	case 8:
		//GraphCut Tracker
//		graph_TrackInit(gcImgCurrentHalf, gImgInitMaskHalf, gRectCurrentHalf);
//		gTkResult.FGImage = graph_GetFGImage();
		break;

	case 9:
		//particle filter tracker
//		particle_TrackInit(gcImgCurrent, gImgInitMask, gRectCurrent);
		gTkResult.FGImage = NULL;
		break;
	//default:
	}	
	gTkResult.FGMask = NULL;
	gTkResult.ObjMask = NULL;

	//initial klt tracker
	klt_TrackInit(gcImgCurrentHalf);

	//display rect
	dis_ShowImgCurrent();	

	//prompt log dir name
	if (gbLogFlag==TRUE && gbReplayFlag==FALSE && gBatchFlag==FALSE){	
		if (dlg_LogDir()==TRUE) 
		{
			//flag to build sub-directory for report log
			gLogDirFlag = TRUE;
		}
		else{
			//cancel log command
			gbLogFlag=FALSE;
		}             
	}

	//init variable
	//strcpy(gClickFrameTitle, iio_GetFileTitle());
	strcpy(gClickFrameTitle, iio_GetImageTitle());
	strcpy(gLastFramePath, iio_GetFilePath());

	//update the blob pos array
	gPosPredict[0].x = (float)(gRectCurrent.left+gRectCurrent.right)/2;
	gPosPredict[0].y = (float)(gRectCurrent.top+gRectCurrent.bottom)/2;
	gPosCount=1;

	//init zoom acumulative variable
	gZoomAccum = 1;
}

///////////////////////////////////////////////////////////////////////////////
void img_ProcessMouseMove(long x, long y)
//Process after click
{
	int i;
	HDC hdc;
	HPEN hpen;

	//repaint image
	AppPaint(hwndCtl, GetDC(hwndCtl));

	hdc = GetDC(hwndCtl);
	hpen = CreatePen(PS_SOLID, giPenWidth, crPenColor);
	SelectObject(hdc, hpen);

	//flip points for display purpose	
	for (i=0; i<giClickCount;i++){			
		if (i==0)
			MoveToEx(hdc, gaClickPoint[0].x, ImageMaxY+ButtonHeight-gaClickPoint[0].y, NULL);
		else
			LineTo(hdc, gaClickPoint[i].x, ImageMaxY+ButtonHeight-gaClickPoint[i].y);		
	}
	//hpen = CreatePen(PS_DOT, 1, RGB(255, 0, 0));
	//SelectObject(hdc, hpen);
	LineTo(hdc, x, y);

	if (giClickCount>=3){	
		LineTo(hdc, gaClickPoint[0].x, ImageMaxY+ButtonHeight-gaClickPoint[0].y);
	}
	
	//release Device Context
	ReleaseDC(hwndCtl, hdc); 
}

///////////////////////////////////////////////////////////////////////////////
void dis_DrawRegion(HWND hwnd, HRGN hRegion)
//display current rect in main window
{
	HBRUSH	hRrush; 
	BOOL	bResult;

	//display track region
	if (gbClickInitFlag==FALSE) return;
	
	hRrush = CreateSolidBrush(crPenColor);
	bResult = FrameRgn(GetDC(hwnd), hRegion, hRrush, giPenWidth,1);

}	

///////////////////////////////////////////////////////////////////////////////
void io_PrintReport()
//Print track report (bottom,left) is origin - obsolete
{
	FILE	*fileout;
	int		row, col;
	BYTE	pixel, threshold;
	double	maxVal=0;

	//build log directory on first entry
	if (gLogDirFlag==TRUE){
		gLogDirFlag = FALSE;

		//log clicked region in .trk file for repeat the same track
		_mkdir(gLogDir);
		strcpy(gLogFile, gLogDir);
		strcat(gLogFile, "\\");
		strcat(gLogFile, gClickFrameTitle);
		gLogFile[strlen(gLogFile)-4] = '\0'; //get rid of suffix
		strcat(gLogFile, ".trk");
		
		fileout = fopen(gLogFile, "w");
		fprintf(fileout, "%s\n", gLastFramePath);
		fprintf(fileout, "%d %d\n", gaClickPoint[0].x, gaClickPoint[0].y);
		fprintf(fileout, "%d %d\n", gaClickPoint[1].x, gaClickPoint[1].y);
		fprintf(fileout, "%d %d\n", gaClickPoint[2].x, gaClickPoint[2].y);
		fprintf(fileout, "%d %d\n", gaClickPoint[3].x, gaClickPoint[3].y);
		fclose(fileout);
	}

	//write rpt file
	if (1){
		//get log file name
		strcpy(gLogFile, gLogDir);
		strcat(gLogFile, "\\");
		strcat(gLogFile, iio_GetFileTitle());
		gLogFile[strlen(gLogFile)-4] = '\0'; //get rid of suffix
		strcat(gLogFile, ".rpt");

		//create/open log file		
		fileout = fopen(gLogFile, "w");

		//write report data
		fprintf(fileout, "%d\n", (long)tReport.time);
		fprintf(fileout, "%s\n", gLastFramePath);
		fprintf(fileout, "%s\n", tReport.framePath);
		fprintf(fileout, "unknown_xvel\n");
		fprintf(fileout, "unknown_yvel\n");
		fprintf(fileout, "unknown_azimuth\n");
		fprintf(fileout, "unknown_grazing\n");
		fprintf(fileout, "unknown_res\n");
		fprintf(fileout, "%d\n", tReport.boxX);
		fprintf(fileout, "%d\n", tReport.boxY);
		fprintf(fileout, "%d\n", tReport.boxWid);
		fprintf(fileout, "%d\n", tReport.boxHgt);

		//write bitmap 	
		cvMinMaxLoc(gImgBitMap, NULL, &maxVal, NULL, NULL, NULL);
		//threshold = max(25, (int)(0.1*maxVal));
		threshold = 0;
		for (row=gRectCurrent.bottom; row>=gRectCurrent.top; row--){			
			for (col=gRectCurrent.left; col<=gRectCurrent.right; col++){
				pixel = (BYTE)cvGetReal2D(gImgBitMap,row,col);				
				if (pixel > threshold)
					fprintf(fileout, "1 ");
				else
					fprintf(fileout, "0 ");
			}
			fprintf(fileout, "\n");
		}

		//write affine matrix		
		fprintf(fileout, "%.10f %.10f %.10f\n", gAffineMatrix[0],gAffineMatrix[1],2*gAffineMatrix[2]);
		fprintf(fileout, "%.10f %.10f %.10f\n", gAffineMatrix[3],gAffineMatrix[4],2*gAffineMatrix[5]);
		
		//write image size
		fprintf(fileout, "%d\n", tReport.imgWidth);
		fprintf(fileout, "%d\n", tReport.imgHeight);

		//close log file
		fclose(fileout);	
	}

	//update last fraem path 
	strcpy(gLastFramePath, tReport.framePath);
}

///////////////////////////////////////////////////////////////////////////////
void io_PrintReport_V20()
//Print track report
{
	FILE	*fileout;
	int		row, col;
	BYTE	pixel, threshold;
	double	maxVal=0;

	//build log directory on first entry
	if (gLogDirFlag==TRUE){
		gLogDirFlag = FALSE;

		//log clicked region in .trk file for repeat the same track
		_mkdir(gLogDir);
		strcpy(gLogFile, gLogDir);
		strcat(gLogFile, "\\");
		strcat(gLogFile, gClickFrameTitle);
		gLogFile[strlen(gLogFile)-4] = '\0'; //get rid of suffix
		strcat(gLogFile, ".trk");
		
		fileout = fopen(gLogFile, "w");
		fprintf(fileout, "%s\n", "V20");
		fprintf(fileout, "%s\n", gLastFramePath);
		fprintf(fileout, "%d %d\n", gaClickPoint[0].x, ImageMaxY-1-gaClickPoint[0].y);
		fprintf(fileout, "%d %d\n", gaClickPoint[1].x, ImageMaxY-1-gaClickPoint[1].y);
		fprintf(fileout, "%d %d\n", gaClickPoint[2].x, ImageMaxY-1-gaClickPoint[2].y);
		fprintf(fileout, "%d %d\n", gaClickPoint[3].x, ImageMaxY-1-gaClickPoint[3].y);
		fclose(fileout);
	}

	//write rpt file
	if (1){
		//get log file name
		strcpy(gLogFile, gLogDir);
		strcat(gLogFile, "\\");
		strcat(gLogFile, iio_GetFileTitle());
		gLogFile[strlen(gLogFile)-4] = '\0'; //get rid of suffix
		strcat(gLogFile, ".rpt");

		//create/open log file		
		fileout = fopen(gLogFile, "w");

		//write report data
		fprintf(fileout, "%d\n", (long)tReport.time);
		fprintf(fileout, "%s\n", gLastFramePath);
		fprintf(fileout, "%s\n", tReport.framePath);
		fprintf(fileout, "unknown_xvel\n");
		fprintf(fileout, "unknown_yvel\n");
		fprintf(fileout, "unknown_azimuth\n");
		fprintf(fileout, "unknown_grazing\n");
		fprintf(fileout, "unknown_res\n");
		fprintf(fileout, "%d\n", tReport.boxX);
		fprintf(fileout, "%d\n", tReport.boxY);
		fprintf(fileout, "%d\n", tReport.boxWid);
		fprintf(fileout, "%d\n", tReport.boxHgt);

		//write bitmap 	
		cvMinMaxLoc(gImgBitMap, NULL, &maxVal, NULL, NULL, NULL);
		//threshold = max(25, (int)(0.1*maxVal));
		threshold = 0;
		for (row=gRectCurrent.bottom; row>=gRectCurrent.top; row--){			
			for (col=gRectCurrent.left; col<=gRectCurrent.right; col++){
				pixel = (BYTE)cvGetReal2D(gImgBitMap,row,col);				
				if (pixel > threshold)
					fprintf(fileout, "1");
				else
					fprintf(fileout, "0");
			}
			fprintf(fileout, "\n");
		}

		//write affine matrix		
		fprintf(fileout, "%.10f %.10f %.10f\n", gAffineMatrix[0],gAffineMatrix[1],2*gAffineMatrix[2]);
		fprintf(fileout, "%.10f %.10f %.10f\n", gAffineMatrix[3],gAffineMatrix[4],2*gAffineMatrix[5]);
		
		//write image size
		fprintf(fileout, "%d\n", tReport.imgWidth);
		fprintf(fileout, "%d\n", tReport.imgHeight);

		//close log file
		fclose(fileout);	
	}

	//update last fraem path 
	strcpy(gLastFramePath, tReport.framePath);
}

///////////////////////////////////////////////////////////////////////////////
void io_PrintReport_V21()
//Print track report
{
	FILE	*fileout;
	int		row, col;
	BYTE	pixel, threshold;
	double	maxVal=0;

	//build log directory on first entry
	if (gLogDirFlag==TRUE){
		gLogDirFlag = FALSE;

		//log clicked region in .trk file for repeat the same track
		_mkdir(gLogDir);
		strcpy(gLogFile, gLogDir);
		strcat(gLogFile, "\\");
		strcat(gLogFile, gClickFrameTitle);
		gLogFile[strlen(gLogFile)-4] = '\0'; //get rid of suffix
		strcat(gLogFile, ".trk");
		
		fileout = fopen(gLogFile, "w");
		fprintf(fileout, "%s\n", "V21");
		fprintf(fileout, "%s\n", gLastFramePath);
		//fprintf(fileout, "%d %d\n", gaClickPoint[0].x, ImageMaxY-1-gaClickPoint[0].y);
		//fprintf(fileout, "%d %d\n", gaClickPoint[1].x, ImageMaxY-1-gaClickPoint[1].y);
		//fprintf(fileout, "%d %d\n", gaClickPoint[2].x, ImageMaxY-1-gaClickPoint[2].y);
		//fprintf(fileout, "%d %d\n", gaClickPoint[3].x, ImageMaxY-1-gaClickPoint[3].y);
		fprintf(fileout, "%d %d\n", gRectInit.left, ImageMaxY-1-gRectInit.top);
		fprintf(fileout, "%d %d\n", gRectInit.right, ImageMaxY-1-gRectInit.top);
		fprintf(fileout, "%d %d\n", gRectInit.right, ImageMaxY-1-gRectInit.bottom);
		fprintf(fileout, "%d %d\n", gRectInit.left, ImageMaxY-1-gRectInit.bottom);

		//output object mask		
		for (row=gRectInit.bottom; row>=gRectInit.top; row--){			
			for (col=gRectInit.left; col<=gRectInit.right; col++){
				pixel = (BYTE)cvGetReal2D(gImgInitMask,row,col);				
				if (pixel > 0)
					fprintf(fileout, "1");
				else
					fprintf(fileout, "0");
			}
			fprintf(fileout, "\n");
		}

		//clean file pointer
		fclose(fileout);
	}

	//write rpt file
	if (1){
		//get log file name
		strcpy(gLogFile, gLogDir);
		strcat(gLogFile, "\\");
		strcat(gLogFile, iio_GetFileTitle());
		gLogFile[strlen(gLogFile)-4] = '\0'; //get rid of suffix
		strcat(gLogFile, ".rpt");

		//create/open log file		
		fileout = fopen(gLogFile, "w");

		//write report data
		fprintf(fileout, "%d\n", (long)tReport.time);
		fprintf(fileout, "%s\n", gLastFramePath);
		fprintf(fileout, "%s\n", tReport.framePath);
		fprintf(fileout, "unknown_xvel\n");
		fprintf(fileout, "unknown_yvel\n");
		fprintf(fileout, "unknown_azimuth\n");
		fprintf(fileout, "unknown_grazing\n");
		fprintf(fileout, "unknown_res\n");
		fprintf(fileout, "%d\n", tReport.boxX);
		fprintf(fileout, "%d\n", tReport.boxY);
		fprintf(fileout, "%d\n", tReport.boxWid);
		fprintf(fileout, "%d\n", tReport.boxHgt);

		//write bitmap 	
		cvMinMaxLoc(gImgBitMap, NULL, &maxVal, NULL, NULL, NULL);
		//threshold = max(25, (int)(0.1*maxVal));
		threshold = 0;
		for (row=gRectCurrent.bottom; row>=gRectCurrent.top; row--){			
			for (col=gRectCurrent.left; col<=gRectCurrent.right; col++){
				pixel = (BYTE)cvGetReal2D(gImgBitMap,row,col);				
				if (pixel > threshold)
					fprintf(fileout, "1");
				else
					fprintf(fileout, "0");
			}
			fprintf(fileout, "\n");
		}

		//write affine matrix		
		fprintf(fileout, "%.10f %.10f %.10f\n", gAffineMatrix[0],gAffineMatrix[1],2*gAffineMatrix[2]);
		fprintf(fileout, "%.10f %.10f %.10f\n", gAffineMatrix[3],gAffineMatrix[4],2*gAffineMatrix[5]);
		
		//write image size
		fprintf(fileout, "%d\n", tReport.imgWidth);
		fprintf(fileout, "%d\n", tReport.imgHeight);

		//close log file
		fclose(fileout);	
	}

	//update last fraem path 
	strcpy(gLastFramePath, tReport.framePath);
}

///////////////////////////////////////////////////////////////////////////////
void io_PrintReport_CID09()
//Print track report in version 2: CID version 0.9
{
	FILE	*fileout;
	int		row, col;
	BYTE	pixel, threshold;
	double	maxVal=0;

	//build log directory on first entry
	if (gLogDirFlag==TRUE){
		gLogDirFlag = FALSE;

		//log clicked region in .trk file for repeat the same track
		_mkdir(gLogDir);
		strcpy(gLogFile, gLogDir);
		strcat(gLogFile, "\\");
		strcat(gLogFile, gClickFrameTitle);
		gLogFile[strlen(gLogFile)-4] = '\0'; //get rid of suffix
		strcat(gLogFile, ".trk");
		
		fileout = fopen(gLogFile, "w");
		fprintf(fileout, "%s\n", "VIVID_CID_ICD_V0.9");
		fprintf(fileout, "%s\n", gLastFramePath);
		fprintf(fileout, "%d %d\n", gaClickPoint[0].x, gaClickPoint[0].y);
		fprintf(fileout, "%d %d\n", gaClickPoint[1].x, gaClickPoint[1].y);
		fprintf(fileout, "%d %d\n", gaClickPoint[2].x, gaClickPoint[2].y);
		fprintf(fileout, "%d %d\n", gaClickPoint[3].x, gaClickPoint[3].y);
		fclose(fileout);
	}

	//write rpt file
	if (1){
		//get log file name
		strcpy(gLogFile, gLogDir);
		strcat(gLogFile, "\\");
		strcat(gLogFile, iio_GetFileTitle());
		gLogFile[strlen(gLogFile)-4] = '\0'; //get rid of suffix
		strcat(gLogFile, ".rpt");

		//create/open log file		
		fileout = fopen(gLogFile, "w");

		//write report data
		fprintf(fileout, "%d\n", (long)tReport.time);		
		fprintf(fileout, "%s\n", tReport.framePath);
		fprintf(fileout, "0\n" );	//unknown_InterlacedFormat
		fprintf(fileout, "0\n");	//unknown_xvel
		fprintf(fileout, "0\n");	//unknown_yvel
		fprintf(fileout, "0\n");	//unknown_AspectAngle
		fprintf(fileout, "0\n");	//unknown_DepressionAngle
		fprintf(fileout, "0\n");	//unknown_SlantRange
		fprintf(fileout, "0\n");	//unknown_resolution
		fprintf(fileout, "0\n");	//unknown_SensorAzimuth
		fprintf(fileout, "0\n");	//unknown_SensorTwist
		fprintf(fileout, "EO\n");	//unknown_SensorModality		
		fprintf(fileout, "%d\n", tReport.boxX);
		fprintf(fileout, "%d\n", tReport.boxY);
		fprintf(fileout, "%d\n", tReport.boxWid);
		fprintf(fileout, "%d\n", tReport.boxHgt);

		//write bitmap 	
		cvMinMaxLoc(gImgBitMap, NULL, &maxVal, NULL, NULL, NULL);
		//threshold = max(25, (int)(0.1*maxVal));
		threshold = 0;
		for (row=gRectCurrent.bottom; row>=gRectCurrent.top; row--){			
			for (col=gRectCurrent.left; col<=gRectCurrent.right; col++){
				pixel = (BYTE)cvGetReal2D(gImgBitMap,row,col);				
				if (pixel > threshold)
					fprintf(fileout, "1 ");
				else
					fprintf(fileout, "0 ");
			}
			fprintf(fileout, "\n");
		}
		//close log file
		fclose(fileout);	
	}

	//output image with box and traceline for debug purpose
	if (0)
	{
		CvScalar pixval;
		IplImage *imgTargetBox;
		char	imgfile[MAX_PATH];
		int		rowDiff, colDiff;
		int		i;
		POINT	pt1, pt2;
		int		row, col;
		
		//get image file name
		strcpy(imgfile, gLogDir);
		strcat(imgfile, "\\");
		strcat(imgfile, iio_GetFileTitle());
		imgfile[strlen(imgfile)-4] = '\0'; //get rid of suffix
		strcat(imgfile, ".jpg");

		//add box to image
		imgTargetBox = cvClone(gcImgCurrent);		
		pixval.val[0] = 0;
		pixval.val[1] = 0;
		pixval.val[2] = 255;
		for (row=gRectCurrent.top;row<=gRectCurrent.bottom;row++){
			rowDiff = min(abs(row-gRectCurrent.top), abs(row-gRectCurrent.bottom));			
			for (col=gRectCurrent.left;col<=gRectCurrent.right;col++){
				colDiff = min(abs(col-gRectCurrent.left), abs(col-gRectCurrent.right));
				if (min(rowDiff, colDiff)>1) continue;
				cvSet2D(imgTargetBox, row, col, pixval);
				cvSet2D(imgTargetBox, row, col, pixval);
			}
		}		

		//add trace line
		for (i=0;i<gPosCount-1;i++){
			pt1.x = (int)gPosPredict[i].x;
			pt1.y = (int)gPosPredict[i].y;
			pt2.x = (int)gPosPredict[i+1].x;
			pt2.y = (int)gPosPredict[i+1].y;
			utl_DrawLine(imgTargetBox, pt1, pt2, pixval); 
		}
		
		if (gbPredictFlag==TRUE)
		{			
			//predict line	in green	
			pixval.val[0] = 0;
			pixval.val[1] = 255;
			pixval.val[2] = 0;
			pt1.x = (int)gPosPredict[0].x;
			pt1.y = (int)gPosPredict[0].y;
			for(i=1;i<30;i++){
				pt2.x = (int)(gPosPredict[0].x + fn_Round(i*gAvgDx));
				pt2.y = (int)(gPosPredict[0].y + fn_Round(i*gAvgDy));
				
				//draw the point if it is not in button area
				utl_DrawLine(imgTargetBox, pt1, pt2, pixval);
				pt1.x = (int)pt2.x;
				pt1.y = (int)pt2.y;
			}
		}
		
		//output image file
		cvFlip(imgTargetBox,NULL,0);
		cvSaveImage(imgfile, imgTargetBox);

		//release memory
		cvRelease(&imgTargetBox);
	}

	//update last fraem path 
	strcpy(gLastFramePath, tReport.framePath);
}

///////////////////////////////////////////////////////////////////////////////
void io_PrintReport_CID10()
//Print track report in version 2: CID version 0.9
{
	FILE	*fileout;
//	int		row, col;
//	BYTE	pixel, threshold;
//	double	maxVal=0;

	//build log directory on first entry
	if (gLogDirFlag==TRUE){
		gLogDirFlag = FALSE;

		//log clicked region in .trk file for repeat the same track
		_mkdir(gLogDir);
		strcpy(gLogFile, gLogDir);
		strcat(gLogFile, "\\");
		strcat(gLogFile, gClickFrameTitle);
		gLogFile[strlen(gLogFile)-4] = '\0'; //get rid of suffix
		strcat(gLogFile, ".trk");
		
		fileout = fopen(gLogFile, "w");
		fprintf(fileout, "%s\n", "VIVID_CID_ICD_V1.0");
		fprintf(fileout, "%s\n", gLastFramePath);
		fprintf(fileout, "%d %d\n", gaClickPoint[0].x, gaClickPoint[0].y);
		fprintf(fileout, "%d %d\n", gaClickPoint[1].x, gaClickPoint[1].y);
		fprintf(fileout, "%d %d\n", gaClickPoint[2].x, gaClickPoint[2].y);
		fprintf(fileout, "%d %d\n", gaClickPoint[3].x, gaClickPoint[3].y);
		fclose(fileout);
	}

	//write rpt file
	if (1){
		//get log file name
		strcpy(gLogFile, gLogDir);
		strcat(gLogFile, "\\");
		strcat(gLogFile, iio_GetFileTitle());
		gLogFile[strlen(gLogFile)-4] = '\0'; //get rid of suffix
		strcat(gLogFile, ".rpt");

		//create/open log file		
		fileout = fopen(gLogFile, "w");

		//write report data
		fprintf(fileout, "%d\n", (long)tReport.time);		
		fprintf(fileout, "%s\n", tReport.framePath);
		fprintf(fileout, "0\n" );	//unknown_InterlacedFormat
		fprintf(fileout, "0\n");	//unknown_xvel
		fprintf(fileout, "0\n");	//unknown_yvel
		fprintf(fileout, "0\n");	//unknown_AspectAngle
		fprintf(fileout, "0\n");	//unknown_DepressionAngle
		fprintf(fileout, "0\n");	//unknown_SlantRange
		fprintf(fileout, "0\n");	//unknown_resolution
		fprintf(fileout, "0\n");	//unknown_SensorAzimuth
		fprintf(fileout, "0\n");	//unknown_SensorTwist
		fprintf(fileout, "0\n");	//unknown_Platform altitude
		fprintf(fileout, "0\n");	//unknown_Platform latitude
		fprintf(fileout, "0\n");	//unknown_Platform longitude
		fprintf(fileout, "EO\n");	//unknown_SensorModality		
		fprintf(fileout, "%d\n", tReport.boxX);
		fprintf(fileout, "%d\n", tReport.boxY);
		fprintf(fileout, "%d\n", tReport.boxWid);
		fprintf(fileout, "%d\n", tReport.boxHgt);

		//close file
		fclose(fileout);	
	}

	//write jpg file with target box for debug purpose
	if (0)
	{
		CvScalar pixval;
		IplImage *imgTargetBox;
		char	imgfile[MAX_PATH];
		int		rowDiff, colDiff;
		//int		i;
		//POINT	pt1, pt2;
		int		row, col;
		
		//get image file name
		strcpy(imgfile, gLogDir);
		strcat(imgfile, "\\");
		strcat(imgfile, iio_GetFileTitle());
		imgfile[strlen(imgfile)-4] = '\0'; //get rid of suffix
		strcat(imgfile, ".jpg");

		//add box to image
		imgTargetBox = cvClone(gcImgCurrent);		
		pixval.val[0] = 0;
		pixval.val[1] = 0;
		pixval.val[2] = 255;
		for (row=gRectCurrent.top;row<=gRectCurrent.bottom;row++){
			rowDiff = min(abs(row-gRectCurrent.top), abs(row-gRectCurrent.bottom));			
			for (col=gRectCurrent.left;col<=gRectCurrent.right;col++){
				colDiff = min(abs(col-gRectCurrent.left), abs(col-gRectCurrent.right));
				if (min(rowDiff, colDiff)>1) continue;
				cvSet2D(imgTargetBox, row, col, pixval);
				cvSet2D(imgTargetBox, row, col, pixval);
			}
		}		
				
		//output image file
		cvFlip(imgTargetBox,NULL,0);
		cvSaveImage(imgfile, imgTargetBox);

		//release memory
		cvRelease(&imgTargetBox);
	}

	//update last fraem path 
	strcpy(gLastFramePath, tReport.framePath);
}

///////////////////////////////////////////////////////////////////////////////
void io_CalcLogDirName(char* outLogDirName)
//get the path of log directory based on video sequence dir name
{
	int i; //loop index
	char index[10];
	char sBufferPath[_MAX_PATH];
	char sLogDirPath[_MAX_PATH];	
	char DirName[30];
	char *sFramePath;

	//get exe path+"tklog"
	strcpy(sLogDirPath, gExeLogPath);
	strcat(sLogDirPath, "\\");
	
	//get video dir name
	sFramePath = iio_GetFilePath();
	iio_CalcFrameDir(sFramePath, DirName);			
	strcpy(outLogDirName, DirName);
	strcat(outLogDirName, "_");		
	strcat(sLogDirPath, outLogDirName);

	//get index
	strcpy(sBufferPath, sLogDirPath);
	for (i=1;i<=10000;i++){
		itoa(i, index, 10);
		strcat(sBufferPath, index);		
		if (_chdir(sBufferPath) != 0){ //directory not exist	
			//return dir path and name			
			strcat(outLogDirName, index);
			break;
		}
		else{//directory exist
			//backup buffer
			strcpy(sBufferPath, sLogDirPath);
		}		
	}			
}

///////////////////////////////////////////////////////////////////////////////
int img_PredictMotion()
//Function: predict motion;
//Input:	gPosPredict array; gPosCount
//output:	gAvgDx; gAvgDy;
{
	float dx, dy;
	float sumDx=0, sumDy=0;
	int i;	
	int count=0;
	int	effectNum;

	//at least two points to predict
	if (gPosCount<gIgnoreFrameNum){
		return 0;
	}

	//effective point count
	effectNum = min(gEffectNum, gPosCount);

	//get avg 
	for (i=gIgnoreFrameNum;i<effectNum;i++){
		//if out of bound, break
		if (gPosPredict[i].x <=0||gPosPredict[i].x>=ImageMaxX-1||gPosPredict[i].y<=0||gPosPredict[i].y>=ImageMaxY-1){
			gPosCount = i;
			break;
		}
		dx = (float)(gPosPredict[i-1].x-gPosPredict[i].x);
		dy = (float)(gPosPredict[i-1].y-gPosPredict[i].y);
		sumDx = sumDx + dx;
		sumDy = sumDy + dy;
		count ++;
	}

	gAvgDx = sumDx /count;
	gAvgDy = sumDy /count;

	return 1;

	//linear interpolation
//	{	
//		float mx, my;
//		float sumx, sumy;
//		float sumNom, sumDenom;
//		float A, B;
//
//		//get avg 
//		sumx = (float)gPosPredict[1].x;
//		sumy = (float)gPosPredict[1].y;
//		for (i=2;i<effectNum;i++){
//			sumx = sumx + gPosPredict[i].x;
//			sumy = sumy + gPosPredict[i].y;
//			count ++;
//		}
//
//		mx = sumx/(count+1);
//		my = sumy/(count+1);
//		
//		//interpolation y=Ax+B; A=sum(xi*(yi-my))/sum(xi*(xi-mx)); B=my-A*mx;
//		sumNom = 0;
//		sumDenom = 0;
//		for (i=1;i<effectNum;i++){				
//			sumNom = sumNom + gPosPredict[i].x*(gPosPredict[i].y - my);
//			sumDenom = sumDenom + gPosPredict[i].x*(gPosPredict[i].x - mx);
//			count ++;
//		}
//
//		A = sumNom/sumDenom;
//		B = my - A*mx;
//	}		
}

///////////////////////////////////////////////////////////////////////////////
void img_CreateRegionMask(HRGN inRegion, IplImage *outImgMask){
	RECT rgnBox;
	BOOL bResult;
	int	 row, col;
	BYTE *maskPixel;

	//get region bound box
	GetRgnBox(inRegion, &rgnBox);

	//set mask
	cvSetZero(outImgMask);	
	for (row=rgnBox.top;row<=rgnBox.bottom;row++){
		for (col=rgnBox.left; col<=rgnBox.right; col++){		
			bResult = PtInRegion(inRegion, col, row);
			if (bResult==TRUE){			
				//set mask image pixel as foreground
				maskPixel = cvPtr2D(outImgMask,row,col,NULL);
				(*maskPixel)=255;
			}			
		}
	}
	//cvSaveImage("result/outImgMask.bmp", outImgMask);
}

///////////////////////////////////////////////////////////////////////////////
void img_RewindVideo()
//rewind video to starting frame and re-init tracker if necessary
{
	giPlayFlag = 0;	
	gbTrackMood=FALSE;
	
	//reset track Rect
	gRectCurrent = gRectInit;			
	
	//rewind to intial frame
	img_StepOneFrame(giFrameInitial);								
	
	//initialize Tracker
	if (gbClickInitFlag==TRUE){	
		giClickCount = 4;
		img_ProcessLClick_Poly();
	}
	
	//reset predict
	gbPredictFlag = FALSE;
	gPosCount=0;
}

///////////////////////////////////////////////////////////////////////////////
void BatchProcess()
//Batch tracking
{
	int		trkIdx;
	int		frameIdx;
	int		rstOpenFile;
	int		rstStep;
	char	sBtkTitle[MAX_PATH];
	char	sBtkPath[MAX_PATH];
	char	sLogPathBackup[MAX_PATH];
	int		iRst;
	char	sLogDirName[_MAX_DIR]; //log dir name
	char	sLogDirPath[MAX_PATH]; //log dir name

	//prompt user to confirm batch tracking
	iRst = DialogBox(hAppInst, MAKEINTRESOURCE(IDD_DIALOG_BATCHCONFIRM), hwndCtl, (DLGPROC)dlg_BatchConfirmProc);
	if (iRst == IDCANCEL){
		return;
	}

	//set tracker parameters	
	gbReplayFlag = FALSE;
	gbTrackMood=TRUE;
	gbLogFlag = TRUE;
	
	//creat batch Log directory
	strcpy(sBtkTitle, iio_GetFileTitle());
	sBtkTitle[strlen(sBtkTitle)-4] = '\0'; //get rid of suffix
	strcpy(sBtkPath, gExePath);
	strcat(sBtkPath, "\\tkLog\\");
	strcat(sBtkPath, sBtkTitle);
	utl_IndexNewDir(sBtkPath);
	_mkdir(sBtkPath);

	//backup original log path
	strcpy(sLogPathBackup, gExeLogPath);
	strcpy(gExeLogPath, sBtkPath);

	//process every .trk file in .btk file
	for(trkIdx=0; trkIdx<gBtkFileCount; trkIdx++) {	
		//read in .trk file
		rstOpenFile = iio_ReadTrackFile(gBtkFileArray[trkIdx]);
		if (rstOpenFile == 0){
			break;
		}

		//update image			
		gcImgCurrent = iio_GetInputImage();
		ImageMaxX = iio_ImageMaxX();
		ImageMaxY = iio_ImageMaxY();
		giFrameInitial = iio_GetFrameNo();

		//HalfSize track image
		cvReleaseImage(&gcImgCurrentHalf);
		gcImgCurrentHalf = cvCreateImage(cvSize(ImageMaxX/2, ImageMaxY/2), 8, 3);
		if (gcImgCurrent!=NULL){			
			cvResize(gcImgCurrent, gcImgCurrentHalf, CV_INTER_LINEAR);
		}

		//mask image
		cvReleaseImage(&gImgInitMaskHalf);
		cvReleaseImage(&gImgInitMask);
		gImgInitMaskHalf = cvCreateImage(cvSize(ImageMaxX/2, ImageMaxY/2), 8, 1);
		gImgInitMask = cvCreateImage(cvSize(ImageMaxX, ImageMaxY), 8, 1);
		//bitmap image
		cvReleaseImage(&gImgBitMap);
		gImgBitMap = cvCreateImage(cvSize(ImageMaxX, ImageMaxY), 8, 1);

		//resize display window
		//AppSetDisplay();
			
		//reset predict
		gbPredictFlag = FALSE;
		gPosCount = 0;			

		//initialize tracker		
		giClickCount=4;
		img_ProcessLClick_Poly();

		//set log path for this sequence
		gLogDirFlag = TRUE;
		io_CalcLogDirName(sLogDirName);
		strcpy(sLogDirPath, sBtkPath);
		strcat(sLogDirPath, "\\");
		strcat(sLogDirPath, sLogDirName);
		strcpy(gLogDir, sLogDirPath);
			
		//track next frame
		frameIdx = 1;
		do {		
			//output track log
			rstStep = img_StepOneFrame(giFrameInitial+frameIdx);
			frameIdx++;						
			
			dis_PrintMessage();
		} while(rstStep==1); //break if stop pressed,
	}

	//restore Log Path
	strcpy(gExeLogPath, sLogPathBackup);
	gbLogFlag = FALSE;
	gBatchFlag = FALSE;

	//finished prompt
	AppSetDisplay();
	dis_ShowImgCurrent();
	DialogBox(hAppInst, MAKEINTRESOURCE(IDD_DIALOG_BATCHFINISH), hwndCtl, (DLGPROC)dlg_BatchFinishProc);
}

///////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK dlg_BatchConfirmProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
//call back function for Log Directory dialog
{
    switch (message) 
    { 				
        case WM_COMMAND: 
            switch (LOWORD(wParam)) 
            { 
                case IDOK: 
					EndDialog(hwndDlg, wParam); 
					return TRUE;
					
                case IDCANCEL: 
                    EndDialog(hwndDlg, wParam); 
                    return FALSE; 
            } 
    } 
    return FALSE; 
}

///////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK dlg_BatchFinishProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
//call back function for Log Directory dialog
{
    switch (message) 
    { 				
        case WM_COMMAND: 
            switch (LOWORD(wParam)) 
            { 
                case IDOK: 
					EndDialog(hwndDlg, wParam); 
					return TRUE;			
            } 
    } 
    return FALSE; 
}

///////////////////////////////////////////////////////////////////////////////
void Batch_Init()
//Batch tracking
{
	char	sBtkTitle[MAX_PATH];
	char	sBtkPath[MAX_PATH];	
	int		iRst;

	//prompt user to confirm batch tracking
	iRst = DialogBox(hAppInst, MAKEINTRESOURCE(IDD_DIALOG_BATCHCONFIRM), hwndCtl, (DLGPROC)dlg_BatchConfirmProc);
	if (iRst == IDCANCEL){
		gBatchFlag = FALSE;
		return;
	}

	//set tracker parameters	
	gbReplayFlag = FALSE;	
	gbLogFlag = TRUE;
	gbBatchStopFlag = FALSE;
	
	//creat batch Log directory
	strcpy(sBtkTitle, iio_GetFileTitle());
	sBtkTitle[strlen(sBtkTitle)-4] = '\0'; //get rid of suffix
	strcpy(sBtkPath, gExePath);
	strcat(sBtkPath, "\\tkLog\\");
	strcat(sBtkPath, sBtkTitle);
	utl_IndexNewDir(sBtkPath);
	_mkdir(sBtkPath);
	strcpy(gBtkPath, sBtkPath);

	//backup original log path
	strcpy(gLogPathBackup, gExeLogPath);
	strcpy(gExeLogPath, sBtkPath);

	//init the first .trk file in .btk file
	Batch_InitTrkFile(gBtkFileArray[0]);
	gTrkIdx=0;		
	gStepCount = 0;
}

///////////////////////////////////////////////////////////////////////////////
void Batch_Exit(boolean flag)
//Batch tracking
{
	//restore Log Path
	strcpy(gExeLogPath, gLogPathBackup);
	gbLogFlag = FALSE;
	gBatchFlag = FALSE;

	//finished prompt
	AppSetDisplay();
	dis_ShowImgCurrent();
	
	//prompt dialog if succeed
	if (flag == TRUE){	
		DialogBox(hAppInst, MAKEINTRESOURCE(IDD_DIALOG_BATCHFINISH), hwndCtl, (DLGPROC)dlg_BatchFinishProc);
	}
}

///////////////////////////////////////////////////////////////////////////////
void Batch_InitTrkFile(char *sTrkFile)
//Batch tracking
{
	int		rstOpenFile;
	char	sLogDirName[_MAX_DIR]; //log dir name
	char	sLogDirPath[MAX_PATH]; //log dir name

	//read in .trk file
	rstOpenFile = iio_ReadTrackFile(sTrkFile);
	if (rstOpenFile == 0){
		Batch_Exit(FALSE);
		return;
	}

	//update image			
	gcImgCurrent = iio_GetInputImage();
	ImageMaxX = iio_ImageMaxX();
	ImageMaxY = iio_ImageMaxY();
	giFrameInitial = iio_GetFrameNo();

	//HalfSize track image
	cvReleaseImage(&gcImgCurrentHalf);
	gcImgCurrentHalf = cvCreateImage(cvSize(ImageMaxX/2, ImageMaxY/2), 8, 3);
	if (gcImgCurrent!=NULL){			
		cvResize(gcImgCurrent, gcImgCurrentHalf, CV_INTER_LINEAR);
	}

	//mask image
	cvReleaseImage(&gImgInitMaskHalf);
	cvReleaseImage(&gImgInitMask);
	gImgInitMaskHalf = cvCreateImage(cvSize(ImageMaxX/2, ImageMaxY/2), 8, 1);
	gImgInitMask = cvCreateImage(cvSize(ImageMaxX, ImageMaxY), 8, 1);
	//bitmap image
	cvReleaseImage(&gImgBitMap);
	gImgBitMap = cvCreateImage(cvSize(ImageMaxX, ImageMaxY), 8, 1);

	//resize display window
	AppSetDisplay();
		
	//reset predict
	gbPredictFlag = FALSE;
	gPosCount = 0;			

	//initialize tracker		
	giClickCount=4;
	img_ProcessLClick_Poly();

	//set log path for this sequence
	gLogDirFlag = TRUE;
	io_CalcLogDirName(sLogDirName);
	strcpy(sLogDirPath, gBtkPath);
	strcat(sLogDirPath, "\\");
	strcat(sLogDirPath, sLogDirName);
	strcpy(gLogDir, sLogDirPath);

	//enable track mood
	gbTrackMood=TRUE;
}