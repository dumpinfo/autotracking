// Hisfortrack.h: interface for the CHisfortrack class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HISFORTRACK_H__57273172_EC0A_4DE1_BE13_FB40672B3E46__INCLUDED_)
#define AFX_HISFORTRACK_H__57273172_EC0A_4DE1_BE13_FB40672B3E46__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BasicDefinitions.h"

class CHisfortrack  
{
public:
	bool TrackNextFrame(unsigned char* inImg, CRECT inRect, TkResult *outResult);
	bool ReleaseHis();
	bool Initial(unsigned char * pImage, char *pMask, CRECT inRect, int nWidth, int nHeight, int nrbins, int ngbins, int nbbins);
	CHisfortrack();
	virtual ~CHisfortrack();

protected:
	double fn_Round(double input);
	void BlobCenterShift(unsigned char * inImg, CRECT *inRect, int range);
	int CalHisCordinates(char pixr, char pixg, char pixb);
	int m_nBakBorder;
	CRECT m_cBounding;
	int m_nImgHeight;
	int m_nImgWidth;
	int m_nWidthStep;
	float * m_pHisBack;
	float * m_pHisFore;
	float * m_pHisInit;
	float * m_pHisRatio;
	unsigned char  * m_pImgBackProj;
	bool m_bInitial;
	int m_iRbins;
	int m_iGbins;
	int m_iBbins;
};

#endif // !defined(AFX_HISFORTRACK_H__57273172_EC0A_4DE1_BE13_FB40672B3E46__INCLUDED_)
