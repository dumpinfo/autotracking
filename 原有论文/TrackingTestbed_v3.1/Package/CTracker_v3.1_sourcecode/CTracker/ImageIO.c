   /*****************************************************************************
 * File:	ImageIO.c
 * Desc:	function of Image IO
 * Author:	Xuhui Zhou  @ Carnegie Mellon University
 * Date:	11/14/2003
 *****************************************************************************/

//#define _MT //enable multithread

#include "ImageIO.h"

#define MAXPATH 128
#define MAXDIR  66
#define MAXEXT  5

static char szReadFilePath[MAXPATH];
static char szSaveFilePath[MAXPATH];
static char szReadFileTitle[MAXPATH];
static char szReadImageTitle[MAXPATH];
static char szSaveFileTitle[MAXPATH];
static char szReadFileDir[MAXDIR];
static char szSaveFileDir[MAXDIR];
static char szReadExt[MAXEXT];
static char szSaveExt[MAXEXT];
static char szBuffer[MAXPATH];

static OPENFILENAME rfn, sfn;
static char szFilterSel[] = "*.trk;*.tks;*.bmp;*.dib;*.jpg;*.jpe;*.png;*.pbm;*.pgm;*.ppm;*.ras;*.tif\0*.trk;*.tks;*.bmp;*.dib;*.jpg;*.jpe;*.png;*.pbm;*.pgm;*.ppm;*.ras;*.tif\0";

static int		gFrameNo=0;
static int		gFrameDigit=1;
IplImage*		gInputImage=NULL;
IplImage*		gMaskImage=NULL;
static int		gImageWidth=720;
static int		gImageHeight=480;

extern POINT	gaClickPoint[4]; //input track region points

static char		gTrkFilePath[MAX_PATH];

char			gBtkFileArray[100][MAX_PATH];
int				gBtkStepNum[100];
int				gBtkFileCount;
int				gRptVerstion=1;

///////////////////////////////////////////////////////////////////////////////
void iio_Init(HWND hwnd)
//define OPENFILENAME rfn
{
	rfn.lpstrTitle = "Pick a sequence or open a logged sequence";
	rfn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	rfn.lStructSize = sizeof(OPENFILENAME);
	rfn.hwndOwner = hwnd;
	rfn.lpstrFilter = szFilterSel;
	rfn.nMaxCustFilter = 0;
	rfn.nFilterIndex = 2l;
	rfn.nMaxFile = MAXPATH;
	rfn.nMaxFileTitle = MAXPATH;
	rfn.lpstrFile = szReadFilePath;
	rfn.lpstrInitialDir = szReadFileDir;
	rfn.lpstrFileTitle = szReadFileTitle;
	rfn.lpstrDefExt = szReadExt;

//	sfn.lpstrTitle = "Save Image in File";
//	sfn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
//	sfn.lStructSize = sizeof(OPENFILENAME);
//	sfn.hwndOwner = hwnd;
//	sfn.lpstrFilter = szFilterSel;
//	sfn.nMaxCustFilter = 0;
//	sfn.nFilterIndex = 2l;
//	sfn.nMaxFile = MAXPATH;
//	sfn.nMaxFileTitle = MAXPATH;
//	sfn.lpstrFile = szSaveFileName;
//	sfn.lpstrInitialDir = szSaveFileDir;
//	sfn.lpstrFileTitle = szSaveFileTitle;
//	sfn.lpstrDefExt = szSaveExt;
}

///////////////////////////////////////////////////////////////////////////////
int iio_OpenFileByPrompt()
//return 0=False; 1=image sequence; 2=trk file.
{	
	int rstOpenFile;
	char suffix[5];
	strcpy(szBuffer, szReadFilePath); //backup current path in case of cancel dialog	
	strcpy(szReadFilePath, "");	


	if(GetOpenFileName(&rfn) == FALSE){//cancel file dialog		
		//restore path
		strcpy(szReadFilePath, szBuffer); 
		rstOpenFile = 0;
	}
	else{	
		//get suffix
		strcpy(suffix, szReadFilePath+strlen(szReadFilePath)-3);
		
		iio_RealeaseMaskImage();
		if (strcmp(suffix, "trk") == 0){		
			//open track file			
			rstOpenFile = iio_ReadTrackFile(szReadFilePath);
		}
		else if (strcmp(suffix, "tks") == 0){
			//open batch track file
			rstOpenFile = iio_ReadBatchTrackFile(szReadFilePath);
		}
		else{//default is a image frame 
			//open image file
			rstOpenFile = iio_ReadFileByName(szReadFilePath);			
		}
	}	
	
	return rstOpenFile;
}

///////////////////////////////////////////////////////////////////////////////
char* iio_GetFileTitle()
{	
	return szReadFileTitle;
}
///////////////////////////////////////////////////////////////////////////////
char* iio_GetImageTitle()
{	
	return szReadImageTitle;
}
///////////////////////////////////////////////////////////////////////////////
char* iio_GetFilePath()
{	
	return szReadFilePath;
}

///////////////////////////////////////////////////////////////////////////////
int iio_GetFrameNo()
{	
	return gFrameNo;
}

///////////////////////////////////////////////////////////////////////////////
IplImage* iio_GetInputImage()
{	
	return gInputImage;
}

///////////////////////////////////////////////////////////////////////////////
IplImage* iio_GetMaskImage()
{	
	return gMaskImage;
}
///////////////////////////////////////////////////////////////////////////////
void iio_RealeaseMaskImage()
{	
	cvReleaseImage(&gMaskImage);	
}

///////////////////////////////////////////////////////////////////////////////
int iio_ImageMaxX()
{	
	return gImageWidth;
}

///////////////////////////////////////////////////////////////////////////////
int iio_ImageMaxY()
{	
	return gImageHeight;
}

///////////////////////////////////////////////////////////////////////////////
int iio_ReadFileByName(char* filepath)
{
	IplImage* inputImage;	

	//if empty string, return	
	if (strlen(filepath)==0) return 0;	

	inputImage = cvLoadImage(filepath, 1);
	if (inputImage==NULL){		
		return 0;
	}
	cvFlip(inputImage,NULL,0);
	iio_CalcFrameNo(filepath, &gFrameNo, &gFrameDigit);	

	//return to global variable
	strcpy(szReadFilePath, filepath);
	iio_CalcFrameTitle(filepath, szReadFileTitle);
	iio_CalcFrameTitle(filepath, szReadImageTitle);
	cvReleaseImage(&gInputImage);	
	gInputImage = inputImage;
	gImageWidth = inputImage->width;
	gImageHeight = inputImage->height;	
	return 1;
}

///////////////////////////////////////////////////////////////////////////////
int iio_ReadTrackFile(char* filepath)
{
	FILE *filein;
	char imgfile[MAXPATH];
	int i;
	int rst;
	int readstatus;
	char firststring[MAXPATH];
	int trkVersion;
	int row, col;
	int minrow, mincol, maxrow, maxcol;
	int maskval;
	char maskchar[2];

	strcpy(gTrkFilePath, filepath);

	filein = fopen(filepath, "r");
	if (filein==NULL) return 0;
	
	strcpy(gTrkFilePath, filepath);

	//read in the first string 
	readstatus = fscanf(filein, "%s", &firststring);
	if (readstatus ==EOF) return 0;

	//judge the version of trk file
	trkVersion = 1; //CMU version 1.0, (bottom, left) is origin
	gRptVerstion = 1;
	if (strcmp(firststring, "VIVID_CID_ICD_V0.9")==0){
		trkVersion = 2;
		gRptVerstion = 2;
	}
	if (strcmp(firststring, "VIVID_CID_ICD_V1.0")==0){
		trkVersion = 3;
		gRptVerstion = 3;
	}	
	if (strcmp(firststring, "V20")==0){ //(top, left) is origin
		trkVersion = 4;
		gRptVerstion = 1;
	}	
	if (strcmp(firststring, "V21")==0){ //with object mask
		trkVersion = 5;
		gRptVerstion = 1;
	}

	//get image file name for different version
	switch(trkVersion) {
	case 1:
		//verstion CMU 1.0
		strcpy(imgfile, firststring);
		break;
	case 2:
		//verstion CID 0.9
		readstatus = fscanf(filein, "%s", &imgfile);
		if (readstatus ==EOF) return 0;
		break;
	case 3:
		//verstion CID 1.0
		readstatus = fscanf(filein, "%s", &imgfile);
		if (readstatus ==EOF) return 0;
		break;
	case 4:
		//verstion CMU 2.0
		readstatus = fscanf(filein, "%s", &imgfile);
		if (readstatus ==EOF) return 0;
		break;	
	case 5:
		//verstion CMU 2.1
		readstatus = fscanf(filein, "%s", &imgfile);
		if (readstatus ==EOF) return 0;
		break;
	}

	//get object blob
	for(i=0;i<4;i++){
		fscanf(filein, "%d", &gaClickPoint[i].x);
		if (readstatus ==EOF) return 0;
		
		fscanf(filein, "%d", &gaClickPoint[i].y);
		if (readstatus ==EOF) return 0;
	}

	//get image
	rst = iio_ReadFileByName(imgfile);
	if (rst==0) return 0;

	//get object mask for version 2.1	
	if (trkVersion==5)
	{       			
		gMaskImage = cvCreateImage(cvSize(gImageWidth, gImageHeight), 8, 1);
		cvSetZero(gMaskImage);
		minrow = min(gaClickPoint[0].y, gaClickPoint[2].y);
		maxrow = max(gaClickPoint[0].y, gaClickPoint[2].y);
		mincol = min(gaClickPoint[0].x, gaClickPoint[2].x);
		maxcol = max(gaClickPoint[0].x, gaClickPoint[2].x);
		for (row=minrow; row<=maxrow; row++){
			for(col=mincol;col<=maxcol;col++){
				//read mask
				fscanf(filein, "%1s", maskchar);
				if (readstatus ==EOF) return 0;
				//set mask image
				maskval = atoi(maskchar);				
				cvSetReal2D(gMaskImage, row, col, maskval*255);
			}
		}
		cvFlip(gMaskImage,NULL,0);
		//cvSaveImage("c:\\CTracker\\gMaskImage.bmp", gMaskImage);
	}	
	fclose(filein);

	//flip blob coordinate for cmu version 2.0
	if (trkVersion==4 || trkVersion==5){
		for(i=0;i<4;i++){
			gaClickPoint[i].y = gImageHeight-1 - gaClickPoint[i].y;			
		}	
	}

	//get trk/rpt file tiltle information
	//iio_CalcFrameNo(gTrkFilePath, &gFrameNo, &gFrameDigit);					
	iio_CalcFrameTitle(gTrkFilePath, szReadFileTitle);

	return 2;
}

///////////////////////////////////////////////////////////////////////////////
int iio_ReadBatchTrackFile(char* filepath)
//return 0=error; 3=success
{
	FILE *filein;
	char trkfile[MAXPATH];
	int i;
	int status;

	filein = fopen(filepath, "r");
	if (filein==NULL) return 0;		

	i=0;
	status = fscanf(filein, "%s", &trkfile);
	while(status!=EOF) {
		strcpy(gBtkFileArray[i], trkfile);
		status = fscanf(filein, "%d", &gBtkStepNum[i]);
		i++;
		status = fscanf(filein, "%s", &trkfile);
	} ;
		
	gBtkFileCount = i;
	return 3;
}

///////////////////////////////////////////////////////////////////////////////
void iio_ReadFileByIndex(int frameIndex)
{
	if (frameIndex<0) return;
	
	iio_ReadNextFrame(szReadFilePath, frameIndex);
}

///////////////////////////////////////////////////////////////////////////////
void iio_ReadNextFrame(char* currentFile, int nextFrameIndex)
{
	char fileNext[150];
	char strFrameNext[10];
	char strFormat[10];	
	int len;

	if (nextFrameIndex<0) return;
	
	strcpy(fileNext, currentFile);
	len = strlen(fileNext);
	wsprintf(strFormat, "%%0%dd", gFrameDigit);
	wsprintf(strFrameNext, strFormat, nextFrameIndex);
		
	strncpy(fileNext+len-4-gFrameDigit, strFrameNext, gFrameDigit);
	
	//Read frame file
	iio_ReadFileByName(fileNext);

	//update variable
	if (gInputImage!=NULL){
		strcpy(szReadFilePath, fileNext);
	}	
}


///////////////////////////////////////////////////////////////////////////////
void iio_CalcFrameNo(char* inFilename, int* outFrameNo, int* outFrameDigit)
//get frame number info from filename
{
	int len = strlen(inFilename);
	int i;	
	char strFrameNo[10];
	int	digitCount;
	
	digitCount = 0;
	for (i=len-5;i>=0;i--){				
		if (inFilename[i]<48 || inFilename[i]>57){			
			break;
		}
		(digitCount)++;
	}

	strncpy(strFrameNo, inFilename+len-4-digitCount, digitCount);
	strFrameNo[digitCount] = '\0';
	
	//output result	
	*outFrameNo = atoi(strFrameNo);
	*outFrameDigit = digitCount;
}

///////////////////////////////////////////////////////////////////////////////
void iio_CalcFrameTitle(char* inFilePath, char* outFrameTitle)
//get frame name from filepath
{
	int len = strlen(inFilePath);
	int i;
	char fileName[50] = "";

	for (i=len-5;i>=0;i--){				
		if (inFilePath[i]==92 || inFilePath[i]==47){ //equal "/" or "\"
			strcpy(fileName, inFilePath+i+1);
			break;
		}
	}

	//output result
	strcpy(outFrameTitle, fileName);
}

///////////////////////////////////////////////////////////////////////////////
void iio_CalcFrameDir(char* inFilePath, char* outFrameDir)
//get frame name from filepath
{
	int len = strlen(inFilePath);
	int i;
	char fileDir[_MAX_PATH] = "";

	strcpy(fileDir, inFilePath);
	
	//get first "/"
	for (i=len-5;i>=0;i--){				
		if (inFilePath[i]==92 || inFilePath[i]==47){ //equal "/" or "\"
			fileDir[i] = '\0';
			break;
		}
	}

	//get second "/"
	for (i=i-1;i>=0;i--){				
		if (inFilePath[i]==92 || inFilePath[i]==47){ //equal "/" or "\"
			strcpy(outFrameDir, fileDir+i+1);
			break;
		}
	}


	//output result
	//strcpy(outFrameDir, fileDir);
}

///////////////////////////////////////////////////////////////////////////////
void iio_Exit()
{
	//release image
	cvReleaseImage(&gInputImage);
}

///////////////////////////////////////////////////////////////////////////////
int iio_ReplayTrkFile()
{	
	if(iio_ReadTrackFile(gTrkFilePath)==0) {
		return 0;
	}
	
	//get trk/rpt file tiltle information
	//iio_CalcFrameNo(gTrkFilePath, &gFrameNo, &gFrameDigit);					
	iio_CalcFrameTitle(gTrkFilePath, szReadFileTitle);
	
	return 1;
}

///////////////////////////////////////////////////////////////////////////////
int iio_ReplayRptFile(int rptIndex)
{
	char	rptFilePath[MAX_PATH];
	char	strFrameNext[10];
	char	strFormat[10];	
	int		len;
	
	FILE	*filein;
	char	imgfile[MAXPATH];
	char	buffer[MAXPATH];
	int		i;	
	int		x, y, width, height;

	int		readstatus;

	if (rptIndex<0) return 0;
	
	if (gInputImage==NULL) return 0;
	
	//get Rpt file path
	strcpy(rptFilePath, gTrkFilePath);
	len = strlen(rptFilePath);
	wsprintf(strFormat, "%%0%dd", gFrameDigit);
	wsprintf(strFrameNext, strFormat, rptIndex);
	
	strncpy(rptFilePath+len-3, "rpt", 3);
	strncpy(rptFilePath+len-4-gFrameDigit, strFrameNext, gFrameDigit);
	
	//Read rpt file
	filein = fopen(rptFilePath, "r");
	if (filein==NULL) return 0;

	switch(gRptVerstion) {
	case 1:
		//CMU Verstion 1.0
		readstatus = fscanf(filein, "%s", &buffer); //frame no
		if (readstatus ==EOF) return 0;
		readstatus = fscanf(filein, "%s", &buffer); //previous frame
		if (readstatus ==EOF) return 0;
		readstatus = fscanf(filein, "%s", &imgfile); //current frame
		if (readstatus ==EOF) return 0;

		for(i=0;i<5;i++){
			readstatus = fscanf(filein, "%s", &buffer);
			if (readstatus ==EOF) return 0;
		}

		readstatus = fscanf(filein, "%d", &x);
		if (readstatus ==EOF) return 0;
		readstatus = fscanf(filein, "%d", &y);		
		y = gInputImage->height - 1 - y;

		if (readstatus ==EOF) return 0;
		readstatus = fscanf(filein, "%d", &width);
		if (readstatus ==EOF) return 0;
		readstatus = fscanf(filein, "%d", &height);
		if (readstatus ==EOF) return 0;

		break;
	case 2:
		//CID Verstion 0.9
		readstatus = fscanf(filein, "%s", &buffer); //frame no		
		if (readstatus ==EOF) return 0;

		readstatus = fscanf(filein, "%s", &imgfile); //current frame
		if (readstatus ==EOF) return 0;

		for(i=0;i<10;i++){
			fscanf(filein, "%s", &buffer);
			if (readstatus ==EOF) return 0;
		}
		
		readstatus = fscanf(filein, "%d", &x);
		if (readstatus ==EOF) return 0;
		readstatus = fscanf(filein, "%d", &y);
		y = gInputImage->height - 1 - y;

		if (readstatus ==EOF) return 0;
		readstatus = fscanf(filein, "%d", &width);
		if (readstatus ==EOF) return 0;		
		readstatus = fscanf(filein, "%d", &height);
		if (readstatus ==EOF) return 0;

		break;
	case 3:
		//CID Verstion 1.0
		readstatus = fscanf(filein, "%s", &buffer); //frame no		
		if (readstatus ==EOF) return 0;

		readstatus = fscanf(filein, "%s", &imgfile); //current frame
		if (readstatus ==EOF) return 0;

		for(i=0;i<13;i++){
			fscanf(filein, "%s", &buffer);
			if (readstatus ==EOF) return 0;
		}
		
		readstatus = fscanf(filein, "%d", &x);
		if (readstatus ==EOF) return 0;
		readstatus = fscanf(filein, "%d", &y);
		y = gInputImage->height - 1 - y;

		if (readstatus ==EOF) return 0;
		readstatus = fscanf(filein, "%d", &width);
		if (readstatus ==EOF) return 0;		
		readstatus = fscanf(filein, "%d", &height);
		if (readstatus ==EOF) return 0;

		break;
	//default:
	}	

	fclose(filein);

	//read image file
	iio_ReadFileByName(imgfile);

	//get rpt file tiltle information
	iio_CalcFrameNo(rptFilePath, &gFrameNo, &gFrameDigit);					
	iio_CalcFrameTitle(rptFilePath, szReadFileTitle);

	//get target box
	gaClickPoint[0].x = x;
	gaClickPoint[0].y = y;
	gaClickPoint[1].x = x+width;
	gaClickPoint[1].y = y;
	gaClickPoint[2].x = x+width;
	gaClickPoint[2].y = y-height;
	gaClickPoint[3].x = x;
	gaClickPoint[3].y = y-height;

	//check errors
	if (x<0||y<0||width<=0 || height<=0){
		return 0;
	}

	return 1;
}