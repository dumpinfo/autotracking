// Hisfortrack.cpp: implementation of the CHisfortrack class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Hisfortrack.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHisfortrack::CHisfortrack()
{
	m_iRbins     = 0;
	m_iGbins     = 0;
	m_iBbins     = 0;
	m_nImgHeight = 0;
	m_nImgWidth  = 0;
	m_nWidthStep = 0;
	m_nBakBorder = 0;
	m_bInitial  = false;
	m_pHisBack     = NULL;
	m_pHisFore     = NULL;
	m_pHisInit     = NULL;
	m_pHisRatio    = NULL;
	m_pImgBackProj = NULL;
	m_cBounding.bottom = 0;
	m_cBounding.left   = 0;
	m_cBounding.top    = 0;
	m_cBounding.right  = 0;
}

CHisfortrack::~CHisfortrack()
{
	if (m_pHisFore)
	{
		delete[] m_pHisFore;
		m_pHisFore = NULL;
	}
	if (m_pHisBack)
	{
		delete[] m_pHisBack;
		m_pHisBack = NULL;
	}
	if (m_pHisInit)
	{
		delete[] m_pHisInit;
		m_pHisInit = NULL;
	}
	if (m_pHisRatio)
	{
		delete[] m_pHisRatio;
		m_pHisRatio = NULL;
	}
	if (m_pImgBackProj)
	{
		delete[] m_pImgBackProj;
		m_pImgBackProj = NULL;
	}
}

bool CHisfortrack::Initial(unsigned char *pImage, char *pMask, CRECT inRect, int nWidth, int nHeight, int nrbins, int ngbins, int nbbins)
{
	if (m_bInitial)
	{
		return false;
	}
	m_bInitial   = true;
	m_cBounding  = inRect;
	m_nImgWidth  = nWidth;
	m_nImgHeight = nHeight;
	m_iBbins     = nbbins;
	m_iGbins     = ngbins;
	m_iRbins     = nrbins;
	if ( nWidth%4==0 )
	{
		m_nWidthStep = nWidth;
	}
	else
	{
		m_nWidthStep = (int(nWidth/4)+1)*4;
	}
	m_pHisBack  = new float[m_iBbins*m_iGbins*m_iRbins];
	m_pHisInit  = new float[m_iBbins*m_iGbins*m_iRbins];
	m_pHisFore  = new float[m_iBbins*m_iGbins*m_iRbins];
	m_pHisRatio = new float[m_iBbins*m_iGbins*m_iRbins];
	m_pImgBackProj = new unsigned char[m_nWidthStep*nHeight];

	memset(m_pHisFore,0,sizeof(float)*m_iBbins*m_iGbins*m_iRbins);
	memset(m_pHisInit,0,sizeof(float)*m_iBbins*m_iGbins*m_iRbins);
	memset(m_pHisBack,0,sizeof(float)*m_iBbins*m_iGbins*m_iRbins);
	memset(m_pHisRatio,0,sizeof(float)*m_iBbins*m_iGbins*m_iRbins);
	memset(m_pImgBackProj, 0, sizeof(unsigned char)*m_nWidthStep*nHeight);

	m_nBakBorder = max((inRect.right-inRect.left)*0.5, (inRect.bottom-inRect.top)*0.5);
	CRECT recttemp;
	recttemp.top    = max(inRect.top-m_nBakBorder, 0);
	recttemp.bottom = min(inRect.bottom+m_nBakBorder, nHeight);
	recttemp.left   = max(inRect.left-m_nBakBorder, 0);
	recttemp.right  = min(inRect.right+m_nBakBorder, nWidth);
	
	unsigned char * pTemp  = NULL;
	unsigned char * pTemp1 = NULL;
	unsigned char pixr = 0;
	unsigned char pixg = 0;
	unsigned char pixb = 0;
	int cortemp = 0;
	double areabk   = (recttemp.bottom - recttemp.top)*(recttemp.right-recttemp.left);
	double areafore = (inRect.bottom - inRect.top)*(inRect.right-inRect.left);
	areabk = areabk-areafore;
	for (int i = recttemp.top; i<recttemp.bottom; i++)
	{
		pTemp = pImage + i*m_nWidthStep*3;
		for (int j = recttemp.left; j<recttemp.right; j++)
		{
			if(i<inRect.top||i>inRect.bottom||j<inRect.left||inRect.right)
			{
				pTemp1 = pTemp + j*3;
				pixr   = (*pTemp1)*m_iRbins/256;
				pixg   = pTemp1[1]*m_iGbins/256;
				pixb   = pTemp1[2]*m_iBbins/256;
				cortemp = CalHisCordinates(pixr, pixg, pixb);
				(*(m_pHisBack + cortemp))+=1/areabk;
			}
		}
	}


	for (i = inRect.top; i<inRect.bottom; i++)
	{
		pTemp = pImage + i*m_nWidthStep*3;
		for (int j = inRect.left; j<inRect.right; j++)
		{
			pTemp1 = pTemp + j*3;
			pixr   = pTemp1[0]*m_iRbins/256;
			pixg   = pTemp1[1]*m_iGbins/256;
			pixb   = pTemp1[2]*m_iBbins/256;
			cortemp = CalHisCordinates(pixr, pixg, pixb);
			(*(m_pHisFore + cortemp))+=1/areafore;
			(*(m_pHisInit + cortemp))+=1/areafore;
		}
	}

	for (i=0; i<m_iRbins; i++)
	{
		for (int j=0; j<m_iGbins;j++)
		{
			for (int z=0; z<m_iBbins; z++)
			{
				cortemp = CalHisCordinates(i, j, z);
				if (m_pHisBack[cortemp] == 0)
				{
					if(m_pHisInit[cortemp]==0)
						m_pHisRatio[cortemp] = 0;
					else
						m_pHisRatio[cortemp] = 2;
				}
				else
				{
					m_pHisRatio[cortemp] = max(0, m_pHisInit[cortemp]/m_pHisBack[cortemp]);
				}
			}
		}
	}

	float minvalue = 255;
	float maxvalue = 0;
	for (i=0; i<m_iRbins; i++)
	{
		for (int j=0; j<m_iGbins;j++)
		{
			for (int z=0; z<m_iBbins; z++)
			{
				cortemp = CalHisCordinates(i, j, z);
				if (m_pHisRatio[cortemp]<minvalue)
				{
					minvalue = m_pHisRatio[cortemp];
				}
				if (m_pHisRatio[cortemp]>maxvalue)
				{
					maxvalue = m_pHisRatio[cortemp];
				}
			}
		}
	}
	
	float gap = 255/(maxvalue-minvalue);
	for (i=0; i<m_iRbins; i++)
	{
		for (int j=0; j<m_iGbins;j++)
		{
			for (int z=0; z<m_iBbins; z++)
			{
				cortemp = CalHisCordinates(i, j, z);
				m_pHisRatio[cortemp] = (m_pHisRatio[cortemp]-minvalue)*gap;
			}
		}
	}


	for (i = recttemp.top; i<recttemp.bottom; i++)
	{
		pTemp  = m_pImgBackProj + i*m_nWidthStep;
		pTemp1 = pImage + i*m_nWidthStep*3;
		for (int j = recttemp.left; j<recttemp.right; j++)
		{
			
			pixr     = pTemp1[j*3]*m_iRbins/256;
			pixg     = pTemp1[1+j*3]*m_iGbins/256;
			pixb     = pTemp1[2+j*3]*m_iBbins/256;
			cortemp  = CalHisCordinates(pixr, pixg, pixb);
			pTemp[j] = m_pHisRatio[cortemp];
		}
	}
}

bool CHisfortrack::ReleaseHis()
{
	if (!m_bInitial)
	{
		return false;
	}
	if (m_pHisFore)
	{
		delete[] m_pHisFore;
		m_pHisFore = NULL;
	}
	if (m_pHisBack)
	{
		delete[] m_pHisBack;
		m_pHisBack = NULL;
	}
	if (m_pHisInit)
	{
		delete[] m_pHisInit;
		m_pHisInit = NULL;
	}
	if (m_pHisRatio)
	{
		delete[] m_pHisRatio;
		m_pHisRatio = NULL;
	}
	if (m_pImgBackProj)
	{
		delete[] m_pImgBackProj;
		m_pImgBackProj = NULL;
	}
	m_iRbins     = 0;
	m_iGbins     = 0;
	m_iBbins     = 0;
	m_nImgHeight = 0;
	m_nImgWidth  = 0;
	m_bInitial = false;
	m_cBounding.bottom = 0;
	m_cBounding.left   = 0;
	m_cBounding.top    = 0;
	m_cBounding.right  = 0;
	return true;
}

bool CHisfortrack::TrackNextFrame(unsigned char *inImg, CRECT inRect, TkResult *outResult)
{
	if (!m_bInitial)
	{
		return false;
	}
	CRECT outerBox;
	float score   = 1;
	unsigned char * pTemp  = NULL;
	unsigned char * pTemp1 = NULL;
	unsigned char pixr     = 0;
	unsigned char pixg     = 0;
	unsigned char pixb     = 0;
	int cortemp = 0;

	inRect.bottom = min(inRect.bottom, m_nImgHeight);
	inRect.left   = max(inRect.left, 0);
	inRect.top    = max(inRect.top,  0);
	inRect.right  = min(inRect.right, m_nImgWidth);

	outerBox.top    = max(inRect.top-m_nBakBorder, 0);
	outerBox.bottom = min(inRect.bottom+m_nBakBorder, m_nImgHeight);
	outerBox.left   = max(inRect.left-m_nBakBorder, 0);
	outerBox.right  = min(inRect.right+m_nBakBorder, m_nImgWidth);

	double areabk   = (outerBox.bottom - outerBox.top)*(outerBox.right-outerBox.left);
	double areafore = (inRect.bottom - inRect.top)*(inRect.right-inRect.left);
	areabk = areabk-areafore;

	memset(m_pHisBack,0,sizeof(float)*m_iBbins*m_iGbins*m_iRbins);
	memset(m_pHisRatio,0,sizeof(float)*m_iBbins*m_iGbins*m_iRbins);
	memset(m_pHisFore,0,sizeof(float)*m_iBbins*m_iGbins*m_iRbins);

	for (int i = outerBox.top; i<outerBox.bottom; i++)
	{
		pTemp = inImg + i*m_nWidthStep*3;
		for (int j = outerBox.left; j<outerBox.right; j++)
		{
			if(i<outerBox.top||i>outerBox.bottom||j<outerBox.left||outerBox.right)
			{
				pTemp1 = pTemp + j*3;
				pixr   = pTemp1[0]*m_iRbins/256;
				pixg   = pTemp1[1]*m_iGbins/256;
				pixb   = pTemp1[2]*m_iBbins/256;
				cortemp = CalHisCordinates(pixr, pixg, pixb);
				(*(m_pHisBack + cortemp))+=1/areabk;
			}
		}
	}

	for (i = inRect.top; i<inRect.bottom; i++)
	{
		pTemp = inImg + i*m_nWidthStep*3;
		for (int j = inRect.left; j<inRect.right; j++)
		{
			pTemp1 = pTemp + j*3;
			pixr   = pTemp1[0]*m_iRbins/256;
			pixg   = pTemp1[1]*m_iGbins/256;
			pixb   = pTemp1[2]*m_iBbins/256;
			cortemp = CalHisCordinates(pixr, pixg, pixb);
			(*(m_pHisFore + cortemp))+=1/areafore;
		}
	}
	
//	for (i=0; i<m_iRbins; i++)
//	{
//		for (int j=0; j<m_iGbins;j++)
//		{
//			for (int z=0; z<m_iBbins; z++)
//			{
//				cortemp = CalHisCordinates(i, j, z);
//				m_pHisFore[cortemp] = (m_pHisFore[cortemp]+m_pHisInit[cortemp])/2.0;
//			}
//		}
//	}

	for (i=0; i<m_iRbins; i++)
	{
		for (int j=0; j<m_iGbins;j++)
		{
			for (int z=0; z<m_iBbins; z++)
			{
				cortemp = CalHisCordinates(i, j, z);
				if (m_pHisBack[cortemp] == 0)
				{
					if(m_pHisInit[cortemp]==0)
						m_pHisRatio[cortemp] = 0;
					else
						m_pHisRatio[cortemp] = 2;
				}
				else
				{
					m_pHisRatio[cortemp] = max(0, m_pHisInit[cortemp]/m_pHisBack[cortemp]);
				}
			}
		}
	}

	float minvalue = 255;
	float maxvalue = 0;
	for (i=0; i<m_iRbins; i++)
	{
		for (int j=0; j<m_iGbins;j++)
		{
			for (int z=0; z<m_iBbins; z++)
			{
				cortemp = CalHisCordinates(i, j, z);
				if (m_pHisRatio[cortemp]<minvalue)
				{
					minvalue = m_pHisRatio[cortemp];
				}
				if (m_pHisRatio[cortemp]>maxvalue)
				{
					maxvalue = m_pHisRatio[cortemp];
				}
			}
		}
	}
	
	float gap = 255/(maxvalue-minvalue);
	for (i=0; i<m_iRbins; i++)
	{
		for (int j=0; j<m_iGbins;j++)
		{
			for (int z=0; z<m_iBbins; z++)
			{
				cortemp = CalHisCordinates(i, j, z);
				m_pHisRatio[cortemp] = (m_pHisRatio[cortemp]-minvalue)*gap;
			}
		}
	}

	memset(m_pImgBackProj, 0, sizeof(unsigned char)*m_nWidthStep*m_nImgHeight);

	double countnum = 0;
	for (i = outerBox.top; i<outerBox.bottom; i++)
	{
		pTemp  = m_pImgBackProj + i*m_nWidthStep;
		pTemp1 = inImg + i*m_nWidthStep*3;
		for (int j = outerBox.left; j<outerBox.right; j++)
		{
			
			pixr     = pTemp1[j*3]*m_iRbins/256;
			pixg     = pTemp1[1+j*3]*m_iGbins/256;
			pixb     = pTemp1[2+j*3]*m_iBbins/256;
			cortemp  = CalHisCordinates(pixr, pixg, pixb);
			pTemp[j] = m_pHisRatio[cortemp];
			if (pTemp[j]>0)
			{
				countnum++;
			}
		}
	}

	BlobCenterShift(m_pImgBackProj, &inRect, 15);


	outResult->targetBox = inRect;
	outResult->FGImage	= m_pImgBackProj;
	outResult->score	= min(1, countnum/(inRect.bottom-inRect.top)/(inRect.right-inRect.left));		
	outResult->occlusion = (score<0.5? true:false );		
	return true;
}

int CHisfortrack::CalHisCordinates(char pixr, char pixg, char pixb)
{
	return (pixb + pixg*m_iBbins + pixr*m_iBbins*m_iGbins);
}

void CHisfortrack::BlobCenterShift(unsigned char *inImg, CRECT *inRect, int range)
{
	CRECT	shiftBox; 
	int		row, col;
	unsigned char	pixel;
	float	sumRow, sumCol, sumPix;
	CPOINT	center, inCenter;
	//int		halfWidth, halfHeight;
	int		dx, dy;
	int		loopCount;	
	unsigned char * pTemp  = NULL;

	//get outer ring box
	shiftBox.left = inRect->left;
	shiftBox.right = inRect->right;
	shiftBox.top = inRect->top;
	shiftBox.bottom = inRect->bottom;

	inCenter.x = (shiftBox.left+shiftBox.right)/2; //input Rect center
	inCenter.y = (shiftBox.top+shiftBox.bottom)/2;
	
	center.x = inCenter.x; //shift center
	center.y = inCenter.y; 

	loopCount=0;
	dx=1;
	dy=1;
	while (abs(dx)+abs(dy)>=1 && loopCount<10)
	{		
		sumRow = 0;
		sumCol = 0;
		sumPix = 0;
		for (row=shiftBox.top; row<shiftBox.bottom; row++)
		{
			pTemp  = m_pImgBackProj + row*m_nWidthStep;
			for (col=shiftBox.left; col<shiftBox.right; col++)
			{
				pixel = pTemp[col];
				sumCol = sumCol + pixel*(col-center.x);
				sumRow = sumRow + pixel*(row-center.y);
				sumPix = sumPix + pixel;
			}
		}
		dx = (int)fn_Round(sumCol/sumPix);
		dy = (int)fn_Round(sumRow/sumPix);		

		//move track box
		shiftBox.left = max(shiftBox.left + dx,0);
		shiftBox.right = min(shiftBox.right + dx, m_nImgWidth);
		shiftBox.top = max(shiftBox.top + dy,0);
		shiftBox.bottom = min(shiftBox.bottom + dy, m_nImgHeight);

		center.x = (shiftBox.left+shiftBox.right)/2;
		center.y = (shiftBox.top+shiftBox.bottom)/2;

		loopCount++;
	}
	//output shift result
	dx = center.x - inCenter.x;
	dy = center.y - inCenter.y;
	if (dx<-range)	dx = -range;
	if (dx>range)	dx = range;
	if (dy<-range)	dy = -range;
	if (dy>range)	dy = range;

	inRect->left	= inRect->left + dx;
	inRect->right	= inRect->right + dx;
	inRect->top		= inRect->top + dy;
	inRect->bottom	= inRect->bottom + dy;
}

double CHisfortrack::fn_Round(double input)
{
	double output, remain;

	output = floor(input);
	remain = input - output;
	if (remain>=0.5) output++;
	
	return((int)output);
}
