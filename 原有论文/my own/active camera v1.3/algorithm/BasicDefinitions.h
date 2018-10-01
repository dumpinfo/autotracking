#ifndef  _BASICDEFINITION_
#define  _BASICDEFINITION_
#include <stdlib.h>
#include <algorithm>
#include <cmath>

#define max(a,b) (((a) > (b)) ? (a) : (b)) 
#define min(a,b) (((a) < (b)) ? (a) : (b)) 

typedef struct tagCRECT
{
	int right;
	int left;
	int bottom;
	int top;
}CRECT;


typedef struct tagTkResult
{
    CRECT		targetBox;
    unsigned char		*FGImage;
	unsigned char		*FGMask;
	unsigned char		*ObjMask;
	float		score;
	bool		occlusion; //flag for occusion
	int			x0; //obj center x
	int			y0; //obj center y
	int			axisA; //ellipse axis A
	int			axisB; //ellipse axis B
	double		angle; //ellipse angle
} TkResult;


typedef struct CPOINT
{
	int x;
	int y;
} CPOINT;
#endif