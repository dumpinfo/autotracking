// MeanShift.cpp: implementation of the CMeanShift class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MeanShift.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMeanShift::CMeanShift()
{
	m_bInitial = false;
	m_bScale   = false;
	m_nImgHeight = 0;
	m_nImgWidth  = 0;
	m_nWidthStep = 0;
	m_nBakBorder = 0;


	memset(m_fModelHist, 0, sizeof(float)*HISTLEN);
	memset(m_fDataHist, 0, sizeof(float)*HISTLEN);

	memset(m_QUANTVALTABLERG,0,256);
	memset(m_QUANTVALTABLEBG,0,256);
	memset(m_QUANTVALTABLERGB,0,256);
	
	m_pIndexImg = NULL;
	m_pRatioImg = NULL;
	m_RG        = NULL;
	m_BG        = NULL;
	m_RGB       = NULL;
}

CMeanShift::~CMeanShift()
{
	if (m_pIndexImg)
	{
		delete m_pIndexImg;
		m_pIndexImg = NULL;
	}
	if (m_pRatioImg)
	{
		delete m_pRatioImg;
		m_pRatioImg = NULL;
	}
	if (m_RG)
	{
		delete m_RG;
		m_RG = NULL;
	}
	if (m_BG)
	{
		delete m_BG;
		m_BG = NULL;
	}
	if (m_RGB)
	{
		delete m_RGB;
		m_RGB = NULL;
	}
}

bool CMeanShift::Initial(unsigned char *pImage, char *pMask, CRECT inRect, int nWidth, int nHeight)
{
	if (m_bInitial)
	{
		return false;
	}

	m_bInitial = true;

	m_cBounding  = inRect;
	m_nImgWidth  = nWidth;
	m_nImgHeight = nHeight;	
	if ( nWidth%4==0 )
	{
		m_nWidthStep = nWidth;
	}
	else
	{
		m_nWidthStep = (int(nWidth/4)+1)*4;
	}


	m_pIndexImg = new unsigned char[m_nWidthStep*m_nImgHeight];
	m_RG        = new unsigned char[m_nWidthStep*m_nImgHeight];
	m_BG        = new unsigned char[m_nWidthStep*m_nImgHeight];
	m_RGB       = new unsigned char[m_nWidthStep*m_nImgHeight];
	m_pRatioImg = new unsigned char[m_nWidthStep*m_nImgHeight];

	memset(m_pIndexImg, 0, sizeof(unsigned char)*m_nWidthStep*m_nImgHeight);
	memset(m_RG       , 0, sizeof(unsigned char)*m_nWidthStep*m_nImgHeight);
	memset(m_BG       , 0, sizeof(unsigned char)*m_nWidthStep*m_nImgHeight);
	memset(m_RGB      , 0, sizeof(unsigned char)*m_nWidthStep*m_nImgHeight);

	memset(m_fModelHist,      0, sizeof(float)*HISTLEN);
	memset(m_fDataHist,       0, sizeof(float)*HISTLEN);
	memset(m_QUANTVALTABLERG, 0, sizeof(unsigned char )*256);
	memset(m_QUANTVALTABLEBG, 0, sizeof(unsigned char )*256);
	memset(m_QUANTVALTABLERGB,0, sizeof(unsigned char )*256);

	unsigned char *pTemp;
	unsigned char *pTempRG;
	unsigned char *pTempBG;
	unsigned char *pTempRGB;

	int RGT[256];
	int BGT[256];
	int RGBT[256];
	memset(RGT,  0, sizeof(int)*256);
	memset(BGT,  0, sizeof(int)*256);
	memset(RGBT, 0, sizeof(int)*256);

	
	for (int row = 0; row<m_nImgHeight; row++)
	{
		pTemp    = pImage + row*m_nWidthStep*3;
		pTempRG  = m_RG   + row*m_nWidthStep;
		pTempBG  = m_BG   + row*m_nWidthStep;
		pTempRGB = m_RGB  + row*m_nWidthStep;
		for (int col = 0; col<m_nImgWidth; col++)
		{
			*pTempBG  = (unsigned char)(((float)*(pTemp+2) - (float)*(pTemp+1) + 255.0)/2.0);
			*pTempRG  = (unsigned char)(((float)*(pTemp+1) - (float)*(pTemp) + 255.0)/2.0);
			*pTempRGB = (unsigned char)(((float)*(pTemp) + (float)*(pTemp+1) + (double)*(pTemp+2))/3.0);

			RGT[(*pTempRG)]++;
			BGT[(*pTempBG)]++;
			RGBT[(*pTempRGB)]++;

			pTemp+=3;
			pTempBG++;
			pTempRG++;
			pTempRGB++;
		}
	}
	
	int cumhist[256];
	
	float accum = 0;
	memset(cumhist, 0, sizeof(int)*256);
	for (int i=1; i < 255; i++)
		cumhist[i] = (accum += RGT[i]);

	m_QUANTVALTABLERG[0] = 0;
	m_QUANTVALTABLERG[255] = 8-1;
	for (i=1; i < 255; i++)
	{
		m_QUANTVALTABLERG[i] = (unsigned char)((8-1)*((float)cumhist[i]/(float)accum)+0.5);	
	}	

	memset(cumhist, 0, sizeof(int)*256);
	accum = 0;
	for ( i=1; i < 255; i++)
		cumhist[i] = (accum += BGT[i]);

	m_QUANTVALTABLEBG[0] = 0;
	m_QUANTVALTABLEBG[255] = 8-1;
	for (i=1; i < 255; i++)
	{
		m_QUANTVALTABLEBG[i] = (unsigned char)((8-1)*((float)cumhist[i]/(float)accum)+0.5);	
	}

	memset(cumhist, 0, sizeof(int)*256);
	accum = 0;
	for ( i=1; i < 255; i++)
		cumhist[i] = (accum += RGBT[i]);

	m_QUANTVALTABLERGB[0] = 0;
	m_QUANTVALTABLERGB[255] = 4-1;
	for (i=1; i < 255; i++)
	{
		m_QUANTVALTABLERGB[i] = (unsigned char)((4-1)*((float)cumhist[i]/(float)accum)+0.5);	
	}


	for (row = 0; row<m_nImgHeight; row++)
	{
		pTempRG  = m_RG + row*m_nWidthStep;
		for (int col = 0; col<m_nImgWidth; col++)
		{
			*pTempRG = m_QUANTVALTABLERG[*pTempRG];
			pTempRG++;
		}
	}

	for (row = 0; row<m_nImgHeight; row++)
	{
		pTempBG  = m_BG + row*m_nWidthStep;
		for (int col = 0; col<m_nImgWidth; col++)
		{
			*pTempBG = m_QUANTVALTABLEBG[*pTempBG];
			pTempBG++;
		}
	}

	for (row = 0; row<m_nImgHeight; row++)
	{
		pTempRGB  = m_RGB + row*m_nWidthStep;
		for (int col = 0; col<m_nImgWidth; col++)
		{
			*pTempRGB = m_QUANTVALTABLERGB[*pTempRGB];
			pTempRGB++;
		}
	}

	for (row = 0; row<m_nImgHeight; row++)
	{
		pTemp    = m_pIndexImg + row*m_nWidthStep;
		pTempRG  = m_RG + row*m_nWidthStep;
		pTempBG  = m_BG + row*m_nWidthStep;
		pTempRGB = m_RGB + row*m_nWidthStep;
		for (int col = 0; col<m_nImgWidth; col++)
		{
			*pTemp = (unsigned char)(((*pTempRGB)*8 + (*pTempRG))*8 + (*pTempBG));
			pTemp++;
			pTempBG++;
			pTempRG++;
			pTempRGB++;
		}
	}
	
	for (row = m_cBounding.top; row<m_cBounding.bottom; row++)
	{
		pTemp    = m_pIndexImg + row*m_nWidthStep+m_cBounding.left;
		for (int col = m_cBounding.left; col<m_cBounding.right; col++)
		{
			m_fModelHist[(*pTemp)]++;
			pTemp++;
		}
	}

	accum=0.0;
	for  (i=0; i<HISTLEN; i++)
	{
		accum += (m_fModelHist[i] * m_fModelHist[i]);
		//accum += gModelhist[i];
	}
	accum = (float)sqrt(accum);

	for  (i=0; i < HISTLEN; i++)
	{	
		m_fModelHist[i] /= accum;
	}

	return true;
}

bool CMeanShift::ReleaseMS()
{
	if (!m_bInitial)
	{
		return false;
	}

	if (m_pIndexImg)
	{
		delete m_pIndexImg;
		m_pIndexImg = NULL;
	}
	if (m_pRatioImg)
	{
		delete m_pRatioImg;
		m_pRatioImg = NULL;
	}
	if (m_RG)
	{
		delete m_RG;
		m_RG = NULL;
	}
	if (m_BG)
	{
		delete m_BG;
		m_BG = NULL;
	}
	if (m_RGB)
	{
		delete m_RGB;
		m_RGB = NULL;
	}
	m_bInitial = false;
	memset(m_fModelHist, 0, sizeof(float)*HISTLEN);
	memset(m_fDataHist,  0, sizeof(float)*HISTLEN);


	memset(m_QUANTVALTABLERG,0,256);
	memset(m_QUANTVALTABLEBG,0,256);
	memset(m_QUANTVALTABLERGB,0,256);
}

bool CMeanShift::TrackNextFrame(unsigned char *pImage, CRECT inRect, TkResult *outResult)
{
	if (!m_bInitial)
	{
		return false;
	}


	int ntimes    = 0;
	int eps       = 1;
	int stepdis   = eps + 1;
	int rowcenter = (inRect.top+inRect.bottom)/2;
	int colcenter = (inRect.left+inRect.right)/2;
	int newrow    = 0;
	int newcol    = 0;
	int deltarow  = 0;
	int deltacol  = 0;
	int halfrow   = (inRect.bottom-inRect.top)/2;
	int halfcol   = (inRect.right-inRect.left)/2;
	int startcol, endcol, startrow, endrow;
	float score, score2;

	startcol = max(colcenter-halfcol,0);
	startrow = max(rowcenter-halfrow,0);
	endrow   = min(m_nImgHeight, rowcenter+halfrow);
	endcol   = min(m_nImgWidth , colcenter+halfcol);

	memset(m_pIndexImg, 0, sizeof(unsigned char)*m_nWidthStep*m_nImgHeight);
	memset(m_RG       , 0, sizeof(unsigned char)*m_nWidthStep*m_nImgHeight);
	memset(m_BG       , 0, sizeof(unsigned char)*m_nWidthStep*m_nImgHeight);
	memset(m_RGB      , 0, sizeof(unsigned char)*m_nWidthStep*m_nImgHeight);


	unsigned char *pTemp;
	unsigned char *pTempRG;
	unsigned char *pTempBG;
	unsigned char *pTempRGB;


	for (int row = 0; row<m_nImgHeight; row++)
	{
		pTemp    = pImage + row*m_nWidthStep*3;
		pTempRG  = m_RG   + row*m_nWidthStep;
		pTempBG  = m_BG   + row*m_nWidthStep;
		pTempRGB = m_RGB  + row*m_nWidthStep;
		for (int col = 0; col<m_nImgWidth; col++)
		{
			*pTempBG  = (unsigned char)(((float)*(pTemp+2) - (float)*(pTemp+1) + 255.0)/2.0);
			*pTempRG  = (unsigned char)(((float)*(pTemp+1) - (float)*(pTemp) + 255.0)/2.0);
			*pTempRGB = (unsigned char)(((float)*(pTemp) + (float)*(pTemp+1) + (float)*(pTemp+2))/3.0);
			pTemp+=3;
			pTempBG++;
			pTempRG++;
			pTempRGB++;
		}
	}

	for (row = 0; row<m_nImgHeight; row++)
	{
		pTempRG  = m_RG + row*m_nWidthStep;
		for (int col = 0; col<m_nImgWidth; col++)
		{
			*pTempRG = m_QUANTVALTABLERG[*pTempRG];
			pTempRG++;
		}
	}

	for (row = 0; row<m_nImgHeight; row++)
	{
		pTempBG  = m_BG + row*m_nWidthStep;
		for (int col = 0; col<m_nImgWidth; col++)
		{
			*pTempBG = m_QUANTVALTABLEBG[*pTempBG];
			pTempBG++;
		}
	}

	for (row = 0; row<m_nImgHeight; row++)
	{
		pTempRGB  = m_RGB + row*m_nWidthStep;
		for (int col = 0; col<m_nImgWidth; col++)
		{
			*pTempRGB = m_QUANTVALTABLERGB[*pTempRGB];
			pTempRGB++;
		}
	}

	for (row = 0; row<m_nImgHeight; row++)
	{
		pTemp    = m_pIndexImg + row*m_nWidthStep;
		pTempRG  = m_RG        + row*m_nWidthStep;
		pTempBG  = m_BG        + row*m_nWidthStep;
		pTempRGB = m_RGB       + row*m_nWidthStep;
		for (int col = 0; col<m_nImgWidth; col++)
		{
			*pTemp = (unsigned char)(((*pTempRGB)*8 + (*pTempRG))*8 + (*pTempBG));
			pTemp++;
			pTempBG++;
			pTempRG++;
			pTempRGB++;
		}
	}

/*	memset(m_fDataHist,  0, sizeof(float)*HISTLEN);
	for (row = startrow; row < endrow; row++)
	{
		pTemp = m_pIndexImg + row*m_nWidthStep + startcol;
		for (int col = startcol; col < endcol; col++)
		{
			m_fDataHist[*pTemp]++;
			pTemp++;
		}
	}

	m_nBakBorder = min((inRect.bottom-inRect.top)/2, (inRect.right-inRect.left)/2);
	CRECT outerbox;
	outerbox.bottom = min(m_nImgHeight, inRect.bottom+m_nBakBorder);
	outerbox.top    = max(0, inRect.top-m_nBakBorder);
	outerbox.left   = max(0, inRect.left-m_nBakBorder);
	outerbox.right  = min(m_nImgWidth, inRect.right+m_nBakBorder);
	CRECT innerbox = inRect;
	
	memset(m_fRatioHist,  0, sizeof(float)*HISTLEN);
	float accum = 0;
	for (int i=0; i < HISTLEN; i++)
		accum += (float)(m_fDataHist[i]*m_fDataHist[i]);
	accum = (float)sqrt(accum);	

	float fminscore = 0; float fmaxscore = 0;
	float fscore;
	for (i=0; i < HISTLEN; i++)
	{ 
		if (m_fDataHist[i] && m_fModelHist[i])	
		{
		    fscore = m_fRatioHist[i] = (float)sqrt(m_fModelHist[i] / (m_fDataHist[i] / accum));
			if (fscore > fmaxscore) fmaxscore = fscore;
			if (fscore < fminscore) fminscore = fscore;
		}
		else
		    m_fRatioHist[i] = 0;


	}	


	memset(m_pRatioImg, 0, sizeof(unsigned char)*m_nWidthStep*m_nImgHeight);
	unsigned char * pTmp;

	for (row = outerbox.top; row<outerbox.bottom; row++)
	{
		pTmp  = m_pRatioImg + row*m_nWidthStep + outerbox.left;
		pTemp = m_pIndexImg + row*m_nWidthStep + outerbox.left;
		for (int col = outerbox.left; col<outerbox.right; col++)
		{
			*pTmp = (unsigned char)((((float)m_fRatioHist[*pTemp] - fminscore)/(fmaxscore-fminscore))*255);
			pTmp++;
			pTemp++;
		}
	}




	float dc      = 0;
	float dr      = 0;
	float sumr    = 0;
	float sumc    = 0;
	accum   = 0;
	float w;
	while (stepdis>=eps && ntimes<10)
	{
		accum = 0;
		

		startcol = max(colcenter-halfcol,0);
		startrow = max(rowcenter-halfrow,0);
		endrow   = min(m_nImgHeight, rowcenter+halfrow);
		endcol   = min(m_nImgWidth , colcenter+halfcol);
		sumr = 0;
		sumc = 0;

		for ( row = startrow; row<endrow; row++)
		{
			pTemp = m_pRatioImg + row*m_nWidthStep + startcol;
			dr = (float)(row-rowcenter);
			for (int col = startcol; col<endcol; col++)
			{
				dc     = (float)(col - colcenter);
				w = *pTemp;
				sumr  += w*dr;
				sumc  += w*dc;
				accum += w;
				pTemp++;
			}
		}

		newrow = (int)(rowcenter + floor(sumr/accum+0.5));
		newcol = (int)(colcenter + floor(sumc/accum+0.5));    
		deltarow = newrow - rowcenter;
		deltacol = newcol - colcenter;
	//    fprintf(stdout,"  step row %d col %d\n",newrow,newcol);

		stepdis = (float)(deltarow*deltarow + deltacol*deltacol);
		rowcenter = newrow; 
		colcenter = newcol;	

		startcol = max(colcenter-halfcol,0);
		startrow = max(rowcenter-halfrow,0);
		endrow   = min(m_nImgHeight, rowcenter+halfrow);
		endcol   = min(m_nImgWidth , colcenter+halfcol); 

		memset(m_fDataHist,  0, sizeof(float)*HISTLEN);
		for (row = startrow; row < endrow; row++)
		{
			pTemp = m_pIndexImg + row*m_nWidthStep + startcol;
			for (int col = startcol; col < endcol; col++)
			{
				m_fDataHist[*pTemp]++;
				pTemp++;
			}
		}

		outerbox.bottom = min(m_nImgHeight, endrow+m_nBakBorder);
		outerbox.top    = max(0, startrow-m_nBakBorder);
		outerbox.left   = max(0, startcol-m_nBakBorder);
		outerbox.right  = min(m_nImgWidth, endcol+m_nBakBorder);
		
		accum = 0;
		for (int i=0; i < HISTLEN; i++)
			accum += (float)(m_fDataHist[i]*m_fDataHist[i]);
		accum = (float)sqrt(accum);	

		fminscore = 0; 
		fmaxscore = 0;

		for (i=0; i < HISTLEN; i++)
		{ 
			if (m_fDataHist[i] && m_fModelHist[i])
			{
			    fscore = m_fRatioHist[i] = (float)sqrt(m_fModelHist[i] / (m_fDataHist[i] / accum));

				if (fscore > fmaxscore) fmaxscore = fscore;
				if (fscore < fminscore) fminscore = fscore;
			}
			else
			    m_fRatioHist[i] = 0;

		}	

		memset(m_pRatioImg, 0, sizeof(unsigned char)*m_nWidthStep*m_nImgHeight);
		unsigned char * pTmp;

		for (row = outerbox.top; row<outerbox.bottom; row++)
		{
			pTmp = m_pRatioImg  + row*m_nWidthStep+outerbox.left;
			pTemp = m_pIndexImg + row*m_nWidthStep+outerbox.left;
			for (int col = outerbox.left; col<outerbox.right; col++)
			{
				*pTmp = (unsigned char)(((m_fRatioHist[*pTemp] - fminscore)/(fmaxscore-fminscore))*255);
				pTmp++;
				pTemp++;
			}
		}
		ntimes++;
	}*/


	


//	outResult->targetBox.bottom = rowcenter + halfrow;
//	outResult->targetBox.top    = rowcenter - halfrow;
//	outResult->targetBox.left   = colcenter - halfcol;
//	outResult->targetBox.right  = colcenter + halfcol;
	CRECT targetbox;
	score = MeanShift(rowcenter, colcenter, halfrow, halfcol, &targetbox);

	inRect = targetbox;
	rowcenter = (inRect.top+inRect.bottom)/2;
	colcenter = (inRect.left+inRect.right)/2;
	halfrow   = (inRect.bottom-inRect.top)/2;
	halfcol   = (inRect.right-inRect.left)/2;
	outResult->targetBox = targetbox;

	if (1)
	{
		int temprow, tempcol;
		temprow = min(1.1*halfrow, halfrow+10);
		tempcol = min(1.1*halfcol, halfcol+10);
		score2 = MeanShift(rowcenter, colcenter, (int)(halfrow+5), (int)(halfcol+5), &targetbox);
		if (score2>score)
		{
			outResult->targetBox = targetbox;
		}
		score = score2;
		temprow = max(0.9*halfrow, halfrow-10);
		tempcol = max(0.9*halfcol, halfcol-10);
		if (temprow>3&&tempcol>3)
		{
			score2 = MeanShift(rowcenter, colcenter, (int)(halfrow-5), (int)(halfcol-5), &targetbox);
			if (score2>score)
			{
				outResult->targetBox = targetbox;
			}
		}

	}
//	outResult->targetBox.top    = (targetbox.bottom + targetbox.top)/2   - halfrow;
//	outResult->targetBox.bottom = (targetbox.bottom + targetbox.top)/2   + halfrow;
//	outResult->targetBox.left   = (targetbox.left   + targetbox.right)/2 - halfcol;
//	outResult->targetBox.right   = (targetbox.left  + targetbox.right)/2 + halfcol;
//	outResult->targetBox = targetbox;
	outResult->FGImage = m_pRatioImg;
	
	return true;
}

float CMeanShift::MeanShift(int rowcenter, int colcenter, int halfrow, int halfcol, CRECT *targetBox)
{
	int ntimes    = 0;
	int eps       = 1;
	int stepdis   = eps + 1;
	int newrow    = 0;
	int newcol    = 0;
	int deltarow  = 0;
	int deltacol  = 0;
	int startcol, endcol, startrow, endrow;

	startcol = max(colcenter-halfcol,0);
	startrow = max(rowcenter-halfrow,0);
	endrow   = min(m_nImgHeight, rowcenter+halfrow);
	endcol   = min(m_nImgWidth , colcenter+halfcol);

	unsigned char *pTemp;
	memset(m_fDataHist,  0, sizeof(float)*HISTLEN);
	for (int row = startrow; row < endrow; row++)
	{
		pTemp = m_pIndexImg + row*m_nWidthStep + startcol;
		for (int col = startcol; col < endcol; col++)
		{
			m_fDataHist[*pTemp]++;
			pTemp++;
		}
	}

	m_nBakBorder = min(halfrow, halfcol);
	
	CRECT outerbox;
	outerbox.bottom = min(m_nImgHeight, endrow+m_nBakBorder);
	outerbox.top    = max(0, startrow-m_nBakBorder);
	outerbox.left   = max(0, startcol-m_nBakBorder);
	outerbox.right  = min(m_nImgWidth, endcol+m_nBakBorder);


	CRECT innerbox;
	innerbox.bottom = rowcenter + halfrow;
	innerbox.top    = rowcenter - halfrow;
	innerbox.right  = colcenter + halfcol;
	innerbox.left   = colcenter - halfcol;
	
	memset(m_fRatioHist,  0, sizeof(float)*HISTLEN);
	float accum = 0;
	for (int i=0; i < HISTLEN; i++)
		accum += (float)(m_fDataHist[i]*m_fDataHist[i]);
	accum = (float)sqrt(accum);	

	float fminscore = 0; float fmaxscore = 0;
	float fscore;
	for (i=0; i < HISTLEN; i++)
	{ 
		if (m_fDataHist[i] && m_fModelHist[i])	
		{
		    fscore = m_fRatioHist[i] = (float)sqrt(m_fModelHist[i] / (m_fDataHist[i] / accum));
			if (fscore > fmaxscore) fmaxscore = fscore;
			if (fscore < fminscore) fminscore = fscore;
		}
		else
		    m_fRatioHist[i] = 0;


	}	


	memset(m_pRatioImg, 0, sizeof(unsigned char)*m_nWidthStep*m_nImgHeight);
	unsigned char * pTmp;

	for (row = outerbox.top; row<outerbox.bottom; row++)
	{
		pTmp  = m_pRatioImg + row*m_nWidthStep + outerbox.left;
		pTemp = m_pIndexImg + row*m_nWidthStep + outerbox.left;
		for (int col = outerbox.left; col<outerbox.right; col++)
		{
			*pTmp = (unsigned char)((((float)m_fRatioHist[*pTemp] - fminscore)/(fmaxscore-fminscore))*255);
			pTmp++;
			pTemp++;
		}
	}




	float dc      = 0;
	float dr      = 0;
	float sumr    = 0;
	float sumc    = 0;
	accum   = 0;
	float w;
	while (stepdis>=eps && ntimes<10)
	{
		accum = 0;
		

		startcol = max(colcenter-halfcol,0);
		startrow = max(rowcenter-halfrow,0);
		endrow   = min(m_nImgHeight, rowcenter+halfrow);
		endcol   = min(m_nImgWidth , colcenter+halfcol);
		sumr = 0;
		sumc = 0;

		for ( row = startrow; row<endrow; row++)
		{
			pTemp = m_pRatioImg + row*m_nWidthStep + startcol;
			dr = (float)(row-rowcenter);
			for (int col = startcol; col<endcol; col++)
			{
				dc     = (float)(col - colcenter);
				w = *pTemp;
				sumr  += w*dr;
				sumc  += w*dc;
				accum += w;
				pTemp++;
			}
		}

		newrow = (int)(rowcenter + floor(sumr/accum+0.5));
		newcol = (int)(colcenter + floor(sumc/accum+0.5));    
		deltarow = newrow - rowcenter;
		deltacol = newcol - colcenter;
	//    fprintf(stdout,"  step row %d col %d\n",newrow,newcol);

		stepdis = (float)(deltarow*deltarow + deltacol*deltacol);
		rowcenter = newrow; 
		colcenter = newcol;	

		startcol = max(colcenter-halfcol,0);
		startrow = max(rowcenter-halfrow,0);
		endrow   = min(m_nImgHeight, rowcenter+halfrow);
		endcol   = min(m_nImgWidth , colcenter+halfcol); 

		memset(m_fDataHist,  0, sizeof(float)*HISTLEN);
		for (row = startrow; row < endrow; row++)
		{
			pTemp = m_pIndexImg + row*m_nWidthStep + startcol;
			for (int col = startcol; col < endcol; col++)
			{
				m_fDataHist[*pTemp]++;
				pTemp++;
			}
		}

		outerbox.bottom = min(m_nImgHeight, endrow+m_nBakBorder);
		outerbox.top    = max(0, startrow-m_nBakBorder);
		outerbox.left   = max(0, startcol-m_nBakBorder);
		outerbox.right  = min(m_nImgWidth, endcol+m_nBakBorder);
		
		accum = 0;
		for (int i=0; i < HISTLEN; i++)
			accum += (float)(m_fDataHist[i]*m_fDataHist[i]);
		accum = (float)sqrt(accum);	

		fminscore = 0; 
		fmaxscore = 0;

		for (i=0; i < HISTLEN; i++)
		{ 
			if (m_fDataHist[i] && m_fModelHist[i])
			{
			    fscore = m_fRatioHist[i] = (float)sqrt(m_fModelHist[i] / (m_fDataHist[i] / accum));

				if (fscore > fmaxscore) fmaxscore = fscore;
				if (fscore < fminscore) fminscore = fscore;
			}
			else
			    m_fRatioHist[i] = 0;

		}	

		memset(m_pRatioImg, 0, sizeof(unsigned char)*m_nWidthStep*m_nImgHeight);
		unsigned char * pTmp;

		for (row = outerbox.top; row<outerbox.bottom; row++)
		{
			pTmp = m_pRatioImg  + row*m_nWidthStep+outerbox.left;
			pTemp = m_pIndexImg + row*m_nWidthStep+outerbox.left;
			for (int col = outerbox.left; col<outerbox.right; col++)
			{
				*pTmp = (unsigned char)(((m_fRatioHist[*pTemp] - fminscore)/(fmaxscore-fminscore))*255);
				pTmp++;
				pTemp++;
			}
		}
		ntimes++;
	}

	targetBox->bottom = rowcenter + halfrow;
	targetBox->top    = rowcenter - halfrow;
	targetBox->left   = colcenter - halfcol;
	targetBox->right  = colcenter + halfcol;
	int inHist[256], outHist[256];


	startcol = max(colcenter-halfcol,0);
	startrow = max(rowcenter-halfrow,0);
	endrow   = min(m_nImgHeight, rowcenter+halfrow);
	endcol   = min(m_nImgWidth , colcenter+halfcol);

	outerbox.bottom = min(m_nImgHeight, endrow+m_nBakBorder);
	outerbox.top    = max(0, startrow-m_nBakBorder);
	outerbox.left   = max(0, startcol-m_nBakBorder);
	outerbox.right  = min(m_nImgWidth, endcol+m_nBakBorder);
		
	memset(inHist,  0, sizeof(int)*256);
	memset(outHist, 0, sizeof(int)*256);
	pTemp = m_pIndexImg;

	for (row = outerbox.top; row<startrow; row++)
	{
		pTemp = m_pIndexImg + row*m_nWidthStep+outerbox.left;
		for (int col = outerbox.left; col<outerbox.right; col++)
		{
			outHist[*pTemp++]++;
		}
	}

	for (row = startrow; row<endrow; row++)
	{
		pTemp = m_pIndexImg + row*m_nWidthStep+outerbox.left;
		for (int col = outerbox.left; col<startcol; col++)
		{
			outHist[*pTemp++]++;
		}
		for (col = startcol; col<endcol; col++)
		{
			inHist[*pTemp++]++;
		}
		for (col = endcol; col<outerbox.right; col++)
		{
			outHist[*pTemp++]++;
		}
	}

	for (row = endrow; row<outerbox.bottom; row++)
	{
		pTemp = m_pIndexImg + row*m_nWidthStep+outerbox.left;
		for (int col = outerbox.left; col<outerbox.right; col++)
		{
			outHist[*pTemp++]++;
		}
	}
	
	int *hiptr, *hoptr, insqlen, outsqlen;
	float   *mptr, indotprod, outdotprod;
	
	fminscore = 99999.9; fmaxscore = -99999.9;

    indotprod = outdotprod = 0.0;
    insqlen = outsqlen = 0;
    for (i=0, hiptr=inHist, hoptr=outHist, mptr=m_fModelHist; i < HISTLEN; i++, hiptr++, hoptr++, mptr++) {
      if (*hiptr) {
		insqlen += (*hiptr) * (*hiptr);
		indotprod += (float)(*hiptr) * (*mptr);
      }
      if (*hoptr) {
		outsqlen += (*hoptr) * (*hoptr);
		outdotprod += (float)(*hoptr) * (*mptr);
      }
    }
	int h = ((halfrow < halfcol) ? halfrow/2 : halfcol/2);    
    fscore = (float)(indotprod/sqrt((float)insqlen) -  outdotprod/sqrt((float)outsqlen));
    if (fscore > fmaxscore) fmaxscore = fscore;
    if (fscore < fminscore) fminscore = fscore;
	int midc = colcenter;
	int sic = midc - halfcol;         if (sic < 0) sic = 0;
    int soc = midc - halfcol - h;     if (soc < 0) soc = 0;
    int eic = midc + halfcol + 1;     if (eic > m_nImgWidth) eic = m_nImgWidth;
    int eoc = midc + halfcol + h + 1; if (eoc > m_nImgWidth) eoc = m_nImgWidth;	

    int midr = rowcenter;
    int sir = midr - halfrow;
    int sor = midr - halfrow - h;
    int eir = midr + halfrow + 1;
    int eor = midr + halfrow + h + 1;
	unsigned char *dsor, *dsir, *deor, *deir;
	dsor = m_pIndexImg + sor * m_nWidthStep + soc;
    deor = m_pIndexImg + eor * m_nWidthStep + soc;
    dsir = m_pIndexImg + sir * m_nWidthStep + sic;
    deir = m_pIndexImg + eir * m_nWidthStep + sic;


	unsigned char * dstart = m_pIndexImg;
	unsigned char * dend = m_pIndexImg + m_nWidthStep * m_nImgWidth;
	pTemp=dsor;
	int col;
	if (dsor > dstart) 
		{
		for ( col=soc; col < eoc; col++, pTemp++) 
		{
		  outdotprod -= m_fModelHist[*pTemp];
		  outsqlen -= (2 * m_fModelHist[*pTemp] - 1);
		  outHist[*pTemp]--;
		}
	}
	if (dsir > dstart) {
		for (pTemp=dsir, col=sic; col < eic; col++, pTemp++)
		{
		  indotprod -= m_fModelHist[*pTemp];
		  insqlen -= (2 * inHist[*pTemp] - 1);
		  inHist[*pTemp]--;
		  outdotprod += m_fModelHist[*pTemp];
		  outsqlen += (2 * outHist[*pTemp] + 1);
		  outHist[*pTemp]++;
		}
	}
	if (deir < dend) 
	{
		for (pTemp=deir, col=sic; col < eic; col++, pTemp++)
		{
		  indotprod += m_fModelHist[*pTemp];
		  insqlen += (2 * inHist[*pTemp] + 1);
		  inHist[*pTemp]++;
		  outdotprod -= m_fModelHist[*pTemp];
		  outsqlen -= (2 * outHist[*pTemp] - 1);
		  outHist[*pTemp]--;
		}
	}
	if (deor < dend) 
	{
		for (pTemp=deor, col=soc; col < eoc; col++, pTemp++) 
		{
		  outdotprod += m_fModelHist[*pTemp];
		  outsqlen += (2 * outHist[*pTemp] + 1);
		  outHist[*pTemp]++;
		}
	}

 	  fscore = (float)(indotprod/sqrt((float)insqlen) -  outdotprod/sqrt((float)outsqlen));
	  if (fscore > fmaxscore) fmaxscore = fscore;
	  if (fscore < fminscore) fminscore = fscore;
	return fmaxscore;
}
