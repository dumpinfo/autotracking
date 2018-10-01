  // BK.h: interface for the BK class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BK_H__BAFF1FE6_7199_4227_A043_96F6840207AF__INCLUDED_)
#define AFX_BK_H__BAFF1FE6_7199_4227_A043_96F6840207AF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

const int K = 3;
const double ALPHA = .05;
const double T = .7;
const double WEIGHT = .05;
const double STD = 35;
const double FACTOR = 2.5;
#include <math.h>

class BK
{
public:
  double weight;                      
  double mean[3];
  double std;
  
  void least(int rgb[3]);
  void adjWeights(BK * pMtch);
  void adjMeanStd(int rgb[3]);
  bool bgd(BK * pMtch);
  void sortGauss(void);

public:
	int label;
	BK();
	virtual ~BK();

};

#endif // !defined(AFX_BK_H__BAFF1FE6_7199_4227_A043_96F6840207AF__INCLUDED_)
