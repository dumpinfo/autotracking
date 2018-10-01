// FeatMS.h: interface for the CFeatMS class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FEATMS_H__EF27E7C2_DDDE_4080_9F94_9D6968287CC0__INCLUDED_)
#define AFX_FEATMS_H__EF27E7C2_DDDE_4080_9F94_9D6968287CC0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BasicDefinitions.h"
#include <search.h>


#define FEATNUM 49
#define BINNUM  32
class CFeatMS  
{
public:
	
	bool TrackNextFrame(unsigned char *pImage, CRECT inRect, TkResult *outResult);
	bool ReleaseMS();
	bool Initial(unsigned char *pImage, char *pMask, CRECT inRect, int nWidth, int nHeight);
	CFeatMS();
	virtual ~CFeatMS();

protected:
	int m_picnum;
	int m_durationforreselect;
	void ReselectFeature(unsigned char *pImage, char *pMask, CRECT inRect);
	int kthSmallest(int  a[], int n, int k);
	double m_MaxFeatScore[3];
	int m_MaxFeatIdx[3];
	void GenerateFeatList();
	int m_nFeatList[FEATNUM][5];
	bool m_bInitial;
	CRECT m_cBounding;
	int m_nImgHeight;
	int m_nImgWidth;
	int m_nWidthStep;
	int m_nBakBorder;
	float * m_pRatioImg;
	unsigned char * m_pWeiImg;
	double m_MaxFeatRatio[3][BINNUM];
	double m_PobjFirst[FEATNUM][BINNUM];
};

#endif // !defined(AFX_FEATMS_H__EF27E7C2_DDDE_4080_9F94_9D6968287CC0__INCLUDED_)
