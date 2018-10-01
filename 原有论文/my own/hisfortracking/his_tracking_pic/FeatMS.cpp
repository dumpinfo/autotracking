// FeatMS.cpp: implementation of the CFeatMS class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FeatMS.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

int compare1(const void *arg1, const void *arg2);


CFeatMS::CFeatMS()
{
	m_bInitial   = false;
	m_nImgWidth  = 0;
	m_nImgHeight = 0;
	m_nWidthStep = 0;
	m_picnum     = 0;
	m_durationforreselect = 10;
	memset(m_nFeatList, sizeof(int), FEATNUM*5);

	m_pRatioImg = NULL;
	m_pWeiImg   = NULL;
	
	memset(m_MaxFeatRatio, 0, sizeof(double)*BINNUM*3);
}

CFeatMS::~CFeatMS()
{
	if (m_pRatioImg)
	{
		delete m_pRatioImg;
		m_pRatioImg = NULL;
	}
	if (m_pWeiImg)
	{
		delete m_pWeiImg;
		m_pWeiImg = NULL;
	}
}

bool CFeatMS::Initial(unsigned char *pImage, char *pMask, CRECT inRect, int nWidth, int nHeight)
{
	if (m_bInitial)
	{
		return false;
	}
	m_bInitial = true;
	m_nImgWidth  = nWidth;
	m_nImgHeight = nHeight;
	m_picnum     = 0;

	if ( nWidth%4==0 )
	{
		m_nWidthStep = nWidth;
	}
	else
	{
		m_nWidthStep = (int(nWidth/4)+1)*4;
	}
	
/////在将来的实验中比较一下用float型和char进行计算的速度和精度的区别
	m_pRatioImg = new float[m_nWidthStep*m_nImgHeight];
	memset(m_pRatioImg, 0, sizeof(float)*m_nWidthStep*m_nImgHeight);

	m_pWeiImg   = new unsigned char[m_nWidthStep*m_nImgHeight];
	memset(m_pWeiImg, 0, sizeof(unsigned char)*m_nWidthStep*m_nImgHeight);

	memset(m_nFeatList, sizeof(int), FEATNUM*5);

	GenerateFeatList();

	int halfwidth  = (inRect.right  - inRect.left)/2;
	int halfheight = (inRect.bottom - inRect.top)/2;
	m_nBakBorder   = max(halfwidth, halfheight);

	CRECT outbox;
	outbox.bottom = min(m_nImgHeight, inRect.bottom+m_nBakBorder);
	outbox.top    = max(0, inRect.top - m_nBakBorder);
	outbox.left   = max(0, inRect.left - m_nBakBorder);
	outbox.right  = min(m_nImgWidth, inRect.right + m_nBakBorder);


	double  Pobj[FEATNUM][BINNUM], Pbg[FEATNUM][BINNUM], Ptotal[FEATNUM][BINNUM];
	memset(Pobj, 0, sizeof(double)*FEATNUM*BINNUM);
	memset(Pbg, 0, sizeof(double)*FEATNUM*BINNUM);
	memset(Ptotal, 0, sizeof(double)*FEATNUM*BINNUM);

	unsigned char * pTemp;
	double feat;
	int col, row, i, j, featbin;
	long countObj=0, countBg=0;
	
	memset(m_MaxFeatRatio, 0, sizeof(double)*BINNUM*3);


	for (row = outbox.top; row < outbox.bottom; row++)
	{
		pTemp = pImage + row*m_nWidthStep*3 + outbox.left*3;
		for (col = outbox.left; col < outbox.right; col++)
		{
			if (row<inRect.bottom && row>=inRect.top && col>=inRect.left && col<inRect.right)
			{
				countObj++;
				for(i=0;i<FEATNUM;i++)
				{
					feat = (int)floor((m_nFeatList[i][0]*pTemp[2]+m_nFeatList[i][1]*pTemp[1]+m_nFeatList[i][2]*pTemp[0]+m_nFeatList[i][3])/m_nFeatList[i][4]);
					featbin = (int)floor(feat/8);
					//object histogram
					Pobj[i][featbin]++;
				}
			}
			else
			{
				countBg++;
				for(i=0;i<FEATNUM;i++)
				{
					feat = (int)floor((m_nFeatList[i][0]*pTemp[2]+m_nFeatList[i][1]*pTemp[1]+m_nFeatList[i][2]*pTemp[0]+m_nFeatList[i][3])/m_nFeatList[i][4]);
					featbin = (int)floor(feat/8);
					//object histogram
					Pbg[i][featbin]++;
				}
			}
			pTemp = pTemp + 3;
		}
	}


	double  x[FEATNUM][BINNUM]; //logRatio
	double  xx[FEATNUM][BINNUM]; //logRatio^2
	double  Ex_obj[FEATNUM], Exx_obj[FEATNUM];
	double  Ex_bg[FEATNUM], Exx_bg[FEATNUM];
	double  Ex_tot[FEATNUM], Exx_tot[FEATNUM];
	double  var_obj[FEATNUM], var_bg[FEATNUM], var_within[FEATNUM], var_between[FEATNUM];
	double  score[FEATNUM], score_sort[FEATNUM];
	double	maxscore, maxscore2, maxscore3;
	int		maxscoreIdx, maxscore2Idx, maxscore3Idx;	//index of max score feature		
	double	*tmp;
	unsigned int		num=FEATNUM;
	double  logRatio[FEATNUM][BINNUM]; 
	memset(logRatio, 0, sizeof(double)*FEATNUM*BINNUM);

	for (i=0;i<FEATNUM;i++)
	{//for each feature
		Ex_obj[i] = 0;
		Exx_obj[i] = 0;
		Ex_bg[i] = 0;
		Exx_bg[i] = 0;
		Ex_tot[i] = 0;
		Exx_tot[i] = 0;						
		for(j=0;j<BINNUM;j++)
		{//for each histogram bin
			Pobj[i][j]        = Pobj[i][j]/countObj;
			m_PobjFirst[i][j] = Pobj[i][j]; //record the histogram of first frame into global array
			Pbg[i][j]         = Pbg[i][j]/countBg;
			Ptotal[i][j]      = (Pobj[i][j] + Pbg[i][j])/2;	
			double temp = min(7,log((Pobj[i][j]+0.001)/(Pbg[i][j]+0.001)));
			logRatio[i][j]    =  max(-7, temp);
			x[i][j]           = logRatio[i][j];
			xx[i][j]          = x[i][j]*x[i][j];
			Ex_obj[i]         = Ex_obj[i] + x[i][j]*Pobj[i][j];
			Exx_obj[i]        = Exx_obj[i] + xx[i][j]*Pobj[i][j];
			Ex_bg[i]          = Ex_bg[i] + x[i][j]*Pbg[i][j];
			Exx_bg[i]         = Exx_bg[i] + xx[i][j]*Pbg[i][j];
			Ex_tot[i]         = Ex_tot[i] + x[i][j]*Ptotal[i][j];
			Exx_tot[i]        = Exx_tot[i] + xx[i][j]*Ptotal[i][j];
		}
	}

	maxscoreIdx = 0;
	for (i=0;i<FEATNUM;i++)
	{
		var_obj[i]     = Exx_obj[i] - Ex_obj[i]*Ex_obj[i];
		var_bg[i]      = Exx_bg[i] - Ex_bg[i]*Ex_bg[i];
		var_between[i] = Exx_tot[i] - Ex_tot[i]*Ex_tot[i];
		var_within[i]  = (var_obj[i] + var_bg[i])/2;
		score[i]       = var_between[i] / max(var_within[i], 1e-6);
		score_sort[i]  = score[i];
		if (i==0)
		{
			maxscore = score[i];
			maxscoreIdx = i;
		}			
		else
		{			
			if(score[i]>maxscore)
			{
				maxscore = score[i];
				maxscoreIdx = i;
			}
		}
	}		

	//get the second max score
	qsort(score_sort, (size_t)FEATNUM, sizeof(double), compare1);
	maxscore2 = score_sort[47];	
	maxscore3 = score_sort[46];
	tmp = (double *)_lfind((void*)&maxscore2, score, &num, sizeof(double), compare1);
	maxscore2Idx = tmp-score;	
	tmp = (double *)_lfind((void*)&maxscore3, score, &num, sizeof(double), compare1);
	maxscore3Idx = tmp-score;

	//record results into global variables for tracking purpose
	m_MaxFeatIdx[0] = maxscoreIdx;
	m_MaxFeatIdx[1] = maxscore2Idx;
	m_MaxFeatIdx[2] = maxscore3Idx;
	m_MaxFeatScore[0] = maxscore;
	m_MaxFeatScore[1] = maxscore2;
	m_MaxFeatScore[2] = maxscore3;

	//record the logratio of max score feature
	for(j=0;j<BINNUM;j++)
	{//for each histogram bin
		m_MaxFeatRatio[0][j] = logRatio[maxscoreIdx][j];
		m_MaxFeatRatio[1][j] = logRatio[maxscore2Idx][j];
		m_MaxFeatRatio[2][j] = logRatio[maxscore3Idx][j];	
	}//end block
	return true;
}

bool CFeatMS::ReleaseMS()
{
	if (!m_bInitial)
	{
		return false;
	}
	m_bInitial   = false;
	m_nImgWidth  = 0;
	m_nImgHeight = 0;
	m_nWidthStep = 0;
	m_picnum     = 0;

	memset(m_nFeatList, 0, sizeof(int)*FEATNUM*5);

	if (m_pRatioImg)
	{
		delete m_pRatioImg;
		m_pRatioImg = NULL;
	}
	if (m_pWeiImg)
	{
		delete m_pWeiImg;
		m_pWeiImg = NULL;
	}
	return true;
}

void CFeatMS::GenerateFeatList()
{
	int r, g, b;
	int featr, featg, featb;
	int tmpr, tmpg, tmpb;
	int	i, j, k, idx; //loop index
	int alphavals[5] = {0, 1, -1, 2, -2};
	int minval, maxval, sumval;
	int sumabs, sumneg;	
	int okflag;
	int featNum;		
	char letters[3] = {'R','G', 'B'};	
	int	 featlist[FEATNUM][5];

	//get feature list
	featNum = 0;
	for(i=0;i<5;i++)
	{
		for(j=0;j<5;j++)
		{
			for(k=0;k<5;k++)
			{
				r = alphavals[i];
				g = alphavals[j];
				b = alphavals[k];
				if (r*r+g*g+b*b>0)
				{
					minval = min(r,min(g,b));
					maxval = max(r,max(g,b));
					sumval = r+g+b;
					if (abs(minval) > abs(maxval))
					{
						r = -r;
						g = -g;
						b = -b;
					}
					else if(sumval<0)
					{
						r = -r;
						g = -g;
						b = -b;
					}
					okflag = 1;
					//test if this feature parallel with exist features
					for (idx=0;idx<featNum;idx++)
					{
						featr = featlist[idx][0];
						featg = featlist[idx][1];
						featb = featlist[idx][2];
						tmpr = g*featb - b*featg;
						tmpg = b*featr - r*featb;
						tmpb = r*featg - g*featr;
						if (tmpr*tmpr+tmpg*tmpg+tmpb*tmpb==0)
						{
							okflag = 0;
							break;
						}
					}
					if (okflag ==1)
					{						
						sumabs = abs(r)+abs(g)+abs(b);
						sumneg = 0;
						if (r<0) sumneg = sumneg +r;
						if (g<0) sumneg = sumneg +g;
						if (b<0) sumneg = sumneg +b;						
	
						featlist[featNum][0] = r;
						featlist[featNum][1] = g;
						featlist[featNum][2] = b;
						featlist[featNum][3] = -256*sumneg;
						featlist[featNum][4] = sumabs;

						featNum = featNum+1;
					}
				}
			}//end b
		}//end g
	}//end r
	for (i=0;i<featNum;i++)
	{
		for(j=0;j<5;j++)
		{
			m_nFeatList[i][j] = featlist[i][j];
		}
	}
}

int compare1(const void *arg1, const void *arg2)
{
   /* Compare two args: */
   double num1 = *(double*)arg1;
   double num2 = *(double*)arg2;

   if(num1> num2){ return 1;}
   else{
	   if(num1<num2) return -1;
	   else return 0;   
   }
}

bool CFeatMS::TrackNextFrame(unsigned char *pImage, CRECT inRect, TkResult *outResult)
{
	if (!m_bInitial)
	{
		return false;
	}

	int i, row, col, featbin, featIdx, ratioIdx, rowcenter, colcenter;
	CRECT outbox;
	int halfrow = (inRect.bottom - inRect.top)/2;
	int halfcol = (inRect.right - inRect.left)/2;
	unsigned char *pTemp;
	float *pTmp, ratio;
	double feat, maxscore;
	m_picnum = (++m_picnum)%m_durationforreselect;

	if (0==m_picnum)
	{
		ReselectFeature(pImage,NULL,outResult->targetBox);
	}	

	m_nBakBorder = max(halfrow, halfcol);

	outbox.bottom = min(inRect.bottom + m_nBakBorder, m_nImgHeight);
	outbox.top    = max(inRect.top - m_nBakBorder, 0);
	outbox.right  = min(inRect.right + m_nBakBorder, m_nImgWidth);
	outbox.left   = max(inRect.left - m_nBakBorder, 0);

	memset(m_pRatioImg, 0, sizeof(float)*m_nWidthStep*m_nImgHeight);
	memset(m_pWeiImg,   0, sizeof(unsigned char)*m_nWidthStep*m_nImgHeight);

	double eps = 1, sumrow, sumcol, sum;
	double dist = eps+1; 
	int ntimes = 0, drow, dcol, rowc[3], colc[3];
	CRECT recttemp = inRect;
	double maxratio=0, minratio = 9999;



	for (i = 0; i<3; i++)
	{
		featIdx  = m_MaxFeatIdx[i];		//max feature index in featlist;
		maxscore = m_MaxFeatScore[i];	//max feature ratio score
		ratioIdx = i;

		rowcenter = (inRect.bottom + inRect.top)/2;
		colcenter = (inRect.left + inRect.right)/2;
		maxratio  = -9999;
		minratio = 9999;

		recttemp.bottom = min(m_nImgHeight, inRect.bottom);
		recttemp.top    = max(0, inRect.top);
		recttemp.left   = max(0, inRect.left);
		recttemp.right  = min(m_nImgWidth, inRect.right);
		
		for (row = outbox.top; row<outbox.bottom; row++)
		{
			pTemp = pImage + row*m_nWidthStep*3 + outbox.left*3;
			pTmp = m_pRatioImg + row*m_nWidthStep + outbox.left;
			for (col = outbox.left; col<outbox.right; col++)
			{
			//calculate feature 
				feat = (m_nFeatList[featIdx][0]*pTemp[2]+m_nFeatList[featIdx][1]*pTemp[1]+m_nFeatList[featIdx][2]*pTemp[0]+m_nFeatList[featIdx][3])/m_nFeatList[featIdx][4];
				featbin = (int)floor(feat/8);
				ratio = m_MaxFeatRatio[ratioIdx][featbin];
				//set ratio image for meanshift purpose
				(*pTmp) = ratio;
				
				if (maxratio<ratio)
				{
					maxratio = ratio;
				}
				if (minratio>ratio)
				{
					minratio = ratio;
				}
				pTmp++;
				pTemp+=3;
			}
		}
		sum = 0; sumcol = 0; sumrow = 0;
		
		dist = eps+1;
		ntimes = 0;		
		while (dist>eps && ntimes<10)
		{
			for (row = recttemp.top; row<recttemp.bottom; row++)
			{
				pTmp = m_pRatioImg + row*m_nWidthStep + recttemp.left;

				drow = row - rowcenter;
				for (col = recttemp.left; col < recttemp.right; col++)
				{
					dcol = col - colcenter;
					sumrow += (*pTmp)*drow;
					sumcol += (*pTmp)*dcol;
					sum    = sum + abs(*pTmp);
					pTmp++;
				}
			}
			if (sum == 0)
			{
				rowc[i] = rowcenter;
				colc[i] = colcenter;
				break; 
			}
			rowcenter += (drow = sumrow/sum);
			colcenter += (dcol = sumcol/sum);
			recttemp.bottom = min(m_nImgHeight, rowcenter + halfrow);
			recttemp.top    = max(0, rowcenter - halfrow);
			recttemp.left   = max(0, colcenter - halfcol);
			recttemp.right  = min(m_nImgWidth,  colcenter+halfcol);
			dist = drow*drow + dcol*dcol;
			ntimes++;

		}
		rowc[i] = rowcenter;
		colc[i] = colcenter;
		if (i==0)
		{
		
			for (row = outbox.top; row<outbox.bottom; row++)
			{
				pTmp  = m_pRatioImg + row*m_nWidthStep + outbox.left;
				pTemp = m_pWeiImg + row*m_nWidthStep + outbox.left; 
				for (col = outbox.left; col<outbox.right; col++)
				{
					(*pTemp) = (unsigned char)(255*((*pTmp)-minratio)/(maxratio-minratio));
					pTmp++;
					pTemp++;
				}
			}
		}
		
	}
	rowcenter = kthSmallest(rowc, 3, 3/2);
	colcenter = kthSmallest(colc, 3, 3/2);
	outResult->targetBox.bottom = rowcenter + halfrow;
	outResult->targetBox.top    = rowcenter - halfrow;
	outResult->targetBox.left   = colcenter - halfcol;
	outResult->targetBox.right  = colcenter + halfcol;
	outResult->FGImage          = m_pWeiImg;
	

}

int CFeatMS::kthSmallest(int a[], int n, int k)
{
   int i,j,l,m ;
    double t, x ;

    l=0 ; m=n-1 ;
    while (l<m) {
        x=a[k] ;
        i=l ;
        j=m ;
        do {
            while (a[i]<x) i++ ;
            while (x<a[j]) j-- ;
            if (i<=j) {
		t=a[i]; a[i]=a[j]; a[j]=t;
                i++ ; j-- ;
            }
        } while (i<=j) ;
        if (j<k) l=i ;
        if (k<i) m=j ;
    }
    return a[k] ;
}

void CFeatMS::ReselectFeature(unsigned char *pImage, char *pMask, CRECT inRect)
{
	int halfwidth  = (inRect.right  - inRect.left)/2;
	int halfheight = (inRect.bottom - inRect.top)/2;
	m_nBakBorder   = max(halfwidth, halfheight);

	CRECT outbox;
	outbox.bottom = min(m_nImgHeight, inRect.bottom+m_nBakBorder);
	outbox.top    = max(0, inRect.top - m_nBakBorder);
	outbox.left   = max(0, inRect.left - m_nBakBorder);
	outbox.right  = min(m_nImgWidth, inRect.right + m_nBakBorder);


	double  Pobj[FEATNUM][BINNUM], Pbg[FEATNUM][BINNUM], Ptotal[FEATNUM][BINNUM];
	memset(Pobj, 0, sizeof(double)*FEATNUM*BINNUM);
	memset(Pbg, 0, sizeof(double)*FEATNUM*BINNUM);
	memset(Ptotal, 0, sizeof(double)*FEATNUM*BINNUM);

	unsigned char * pTemp;
	double feat;
	int col, row, i, j, featbin;
	long countObj=0, countBg=0;
	
	memset(m_MaxFeatRatio, 0, sizeof(double)*BINNUM*3);


	for (row = outbox.top; row < outbox.bottom; row++)
	{
		pTemp = pImage + row*m_nWidthStep*3 + outbox.left*3;
		for (col = outbox.left; col < outbox.right; col++)
		{
			if (row<inRect.bottom && row>=inRect.top && col>=inRect.left && col<inRect.right)
			{
				countObj++;
				for(i=0;i<FEATNUM;i++)
				{
					feat = (int)floor((m_nFeatList[i][0]*pTemp[2]+m_nFeatList[i][1]*pTemp[1]+m_nFeatList[i][2]*pTemp[0]+m_nFeatList[i][3])/m_nFeatList[i][4]);
					featbin = (int)floor(feat/8);
					//object histogram
					Pobj[i][featbin]++;
				}
			}
			else
			{
				countBg++;
				for(i=0;i<FEATNUM;i++)
				{
					feat = (int)floor((m_nFeatList[i][0]*pTemp[2]+m_nFeatList[i][1]*pTemp[1]+m_nFeatList[i][2]*pTemp[0]+m_nFeatList[i][3])/m_nFeatList[i][4]);
					featbin = (int)floor(feat/8);
					//object histogram
					Pbg[i][featbin]++;
				}
			}
			pTemp = pTemp + 3;
		}
	}


	double  x[FEATNUM][BINNUM]; //logRatio
	double  xx[FEATNUM][BINNUM]; //logRatio^2
	double  Ex_obj[FEATNUM], Exx_obj[FEATNUM];
	double  Ex_bg[FEATNUM], Exx_bg[FEATNUM];
	double  Ex_tot[FEATNUM], Exx_tot[FEATNUM];
	double  var_obj[FEATNUM], var_bg[FEATNUM], var_within[FEATNUM], var_between[FEATNUM];
	double  score[FEATNUM], score_sort[FEATNUM];
	double	maxscore, maxscore2, maxscore3;
	int		maxscoreIdx, maxscore2Idx, maxscore3Idx;	//index of max score feature		
	double	*tmp;
	unsigned int		num=FEATNUM;
	double  logRatio[FEATNUM][BINNUM]; 
	memset(logRatio, 0, sizeof(double)*FEATNUM*BINNUM);

	for (i=0;i<FEATNUM;i++)
	{//for each feature
		Ex_obj[i] = 0;
		Exx_obj[i] = 0;
		Ex_bg[i] = 0;
		Exx_bg[i] = 0;
		Ex_tot[i] = 0;
		Exx_tot[i] = 0;						
		for(j=0;j<BINNUM;j++)
		{//for each histogram bin
			Pobj[i][j]        = Pobj[i][j]/countObj;
			m_PobjFirst[i][j] = Pobj[i][j]; //record the histogram of first frame into global array
			Pbg[i][j]         = Pbg[i][j]/countBg;
			Ptotal[i][j]      = (Pobj[i][j] + Pbg[i][j])/2;	
			double temp = min(7,log((Pobj[i][j]+0.001)/(Pbg[i][j]+0.001)));
			logRatio[i][j]    =  max(-7, temp);
			x[i][j]           = logRatio[i][j];
			xx[i][j]          = x[i][j]*x[i][j];
			Ex_obj[i]         = Ex_obj[i] + x[i][j]*Pobj[i][j];
			Exx_obj[i]        = Exx_obj[i] + xx[i][j]*Pobj[i][j];
			Ex_bg[i]          = Ex_bg[i] + x[i][j]*Pbg[i][j];
			Exx_bg[i]         = Exx_bg[i] + xx[i][j]*Pbg[i][j];
			Ex_tot[i]         = Ex_tot[i] + x[i][j]*Ptotal[i][j];
			Exx_tot[i]        = Exx_tot[i] + xx[i][j]*Ptotal[i][j];
		}
	}

	maxscoreIdx = 0;
	for (i=0;i<FEATNUM;i++)
	{
		var_obj[i]     = Exx_obj[i] - Ex_obj[i]*Ex_obj[i];
		var_bg[i]      = Exx_bg[i] - Ex_bg[i]*Ex_bg[i];
		var_between[i] = Exx_tot[i] - Ex_tot[i]*Ex_tot[i];
		var_within[i]  = (var_obj[i] + var_bg[i])/2;
		score[i]       = var_between[i] / max(var_within[i], 1e-6);
		score_sort[i]  = score[i];
		if (i==0)
		{
			maxscore = score[i];
			maxscoreIdx = i;
		}			
		else
		{			
			if(score[i]>maxscore)
			{
				maxscore = score[i];
				maxscoreIdx = i;
			}
		}
	}		

	//get the second max score
	qsort(score_sort, (size_t)FEATNUM, sizeof(double), compare1);
	maxscore2 = score_sort[47];	
	maxscore3 = score_sort[46];
	tmp = (double *)_lfind((void*)&maxscore2, score, &num, sizeof(double), compare1);
	maxscore2Idx = tmp-score;	
	tmp = (double *)_lfind((void*)&maxscore3, score, &num, sizeof(double), compare1);
	maxscore3Idx = tmp-score;

	//record results into global variables for tracking purpose
	m_MaxFeatIdx[0] = maxscoreIdx;
	m_MaxFeatIdx[1] = maxscore2Idx;
	m_MaxFeatIdx[2] = maxscore3Idx;
	m_MaxFeatScore[0] = maxscore;
	m_MaxFeatScore[1] = maxscore2;
	m_MaxFeatScore[2] = maxscore3;

	//record the logratio of max score feature
	for(j=0;j<BINNUM;j++)
	{//for each histogram bin
		m_MaxFeatRatio[0][j] = logRatio[maxscoreIdx][j];
		m_MaxFeatRatio[1][j] = logRatio[maxscore2Idx][j];
		m_MaxFeatRatio[2][j] = logRatio[maxscore3Idx][j];	
	}//end block
}
