// MeanShift.h: interface for the CMeanShift class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MEANSHIFT_H__300A30DF_9A28_4D33_BCC2_7C8E6533E196__INCLUDED_)
#define AFX_MEANSHIFT_H__300A30DF_9A28_4D33_BCC2_7C8E6533E196__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "BasicDefinitions.h"
#define HISTLEN 8*8*4

class CMeanShift  
{
public:
	bool TrackNextFrame(unsigned char *inImg, CRECT inRect, TkResult *outResult);
	bool ReleaseMS();
	bool Initial(unsigned char *pImage, char *pMask, CRECT inRect, int nWidth, int nHeight);
	CMeanShift();
	virtual ~CMeanShift();

protected:
	bool m_bScale;
	float MeanShift(int rowcenter, int colcenter, int halfrow, int halfcol, CRECT *targetBox);
	bool m_bInitial;
	CRECT m_cBounding;
	int m_nImgHeight;
	int m_nImgWidth;
	int m_nWidthStep;
	int m_nBakBorder;
	float m_fModelHist[HISTLEN];
	float m_fDataHist[HISTLEN];
	float m_fRatioHist[HISTLEN];
	
	unsigned char * m_pIndexImg;
	unsigned char *m_RG;
	unsigned char *m_BG;
	unsigned char *m_RGB;
	unsigned char * m_pRatioImg;
	unsigned char m_QUANTVALTABLERGB[256];
	unsigned char m_QUANTVALTABLERG[256];
	unsigned char m_QUANTVALTABLEBG[256];
};

#endif // !defined(AFX_MEANSHIFT_H__300A30DF_9A28_4D33_BCC2_7C8E6533E196__INCLUDED_)
