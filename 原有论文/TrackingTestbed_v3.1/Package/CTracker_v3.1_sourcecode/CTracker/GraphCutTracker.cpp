 /*****************************************************************************
 * File:	GraphCutWrap.cpp
 * Desc:	functions of GraphCut Tracking Algorithm
 * Author:	Xuhui Zhou  @ Carnegie Mellon University
 * Date:	03/01/2006
 *****************************************************************************/

#include "GraphCutTracker.h"
#include "graph.h"

IplImage	*gImgPrevFrame; 
IplImage	*gImg1FGMask;

///////////////////////////////////////////////////////////////////////////////
void graph_TrackCleanUp()
//release memory
{
	//projection image
	if (gImgPrevFrame) cvReleaseImage(&gImgPrevFrame);
	if (gImg1FGMask) cvReleaseImage(&gImg1FGMask);
}

///////////////////////////////////////////////////////////////////////////////
void graph_TrackInit(IplImage* inImage, IplImage* inMask, RECT inTargetBox)
//tracker init with box and mask
{
	gImgPrevFrame	= cvCloneImage(inImage);
	gImg1FGMask		= cvCloneImage(inMask);	
	
}

//////////////////////////////////////////////////////////////////////////////
void graph_TrackNextFrame(IplImage* inImage, RECT inStartBox, TkResult *outResult)
//track one frame
{		
	int			xi, yi;	
	CvScalar	scaPixel;
	double		dPixelMask;	
	RECT		rectWindow;

	////build Graph
	Graph::node_id	nodes[400*300*2];
	int				xn, yn;
	CvScalar		scaPixelNeighbor;
	Graph::captype	weightFG;
	Graph::captype	weightBG;
	Graph::captype	weightEdge;
	int				nodeIdx;
	int				neighborIdx;		
	double			sigma;
	int				i,j;
	double			dist;
	int				border;
	int				graphOffset;	
	int				areaWindow;
	Graph*			pGraph		= new Graph();		
	CvSize			sizeWindow;

	border = 20;
	rectWindow.left		= max(0,inStartBox.left - border);
	rectWindow.right	= min(inImage->width-1,inStartBox.right + border);
	rectWindow.top		= max(0,inStartBox.top - border);
	rectWindow.bottom	= min(inImage->height-1,inStartBox.bottom + border);	

	sizeWindow.width	= rectWindow.right-rectWindow.left+1;
	sizeWindow.height	= rectWindow.bottom-rectWindow.top+1;
	areaWindow			= sizeWindow.width*sizeWindow.height;		

	sigma = 10;		
	//build node
	for(xi=0;xi<sizeWindow.width;xi++){
		for(yi=0;yi<sizeWindow.height;yi++){	
			graphOffset = yi*sizeWindow.width+xi;

			//previous (first) frame nodes
			neighborIdx = graphOffset;
			nodes[neighborIdx] = pGraph -> add_node();

			//current (second) frame nodes
			nodeIdx = areaWindow+graphOffset;
			nodes[nodeIdx] = pGraph -> add_node();
		}
	}

	//build link	
	for(xi=0;xi<sizeWindow.width;xi++){
		for(yi=0;yi<sizeWindow.height;yi++){		
			graphOffset = yi*sizeWindow.width+xi;

			//get FG terminal weights			
			dPixelMask = cvGetReal2D(gImg1FGMask, yi+rectWindow.top, xi+rectWindow.left);
			if (dPixelMask>0){				
				weightFG = (Graph::captype)5;
				weightBG = 0;
			}
			else{
				weightFG = 0;
				weightBG = (Graph::captype)5;
			}			

			//Add terminal weights on previous frame nodes
			neighborIdx = graphOffset;
			pGraph -> set_tweights(nodes[neighborIdx], weightFG, weightBG);

			//Add neighbor-link edge across frame, neighbor here is previous frame pixel
			scaPixel		= cvGet2D(inImage, yi+rectWindow.top, xi+rectWindow.left);			
			scaPixelNeighbor= cvGet2D(gImgPrevFrame, yi+rectWindow.top, xi+rectWindow.left); 			
			weightEdge		= (Graph::captype)(exp(-0.5*pow((scaPixelNeighbor.val[0]-scaPixel.val[0])/sigma,2)));
			nodeIdx			= areaWindow+graphOffset;
			pGraph -> add_edge(nodes[nodeIdx], nodes[neighborIdx], weightEdge, weightEdge);

			//Add neighbor edge within current frame			
			for(i=-1;i<=1;i++){
				for(j=-1;j<=1;j++){
					//skip self
					if (i==0 && j==0) continue;

					xn	= xi+i;
					yn	= yi+j;
					//check boundary
					if (xn<0||xn>sizeWindow.width-1||yn<0||yn>sizeWindow.height-1) continue;
					if (xn+rectWindow.left<0||xn+rectWindow.left>inImage->width-1||yn+rectWindow.top<0||yn+rectWindow.top>inImage->height-1) continue;

					dist			= abs(i) + abs(j);
					if (dist>1)		continue;

					scaPixelNeighbor= cvGet2D(inImage, yn+rectWindow.top, xn+rectWindow.left);					
					//weightEdge	= (Graph::captype)(exp(-0.5*pow((scaPixelNeighbor.val[0]-scaPixel.val[0])/sigma,2))/dist);
					weightEdge		= (Graph::captype)(exp(-0.5*pow((scaPixelNeighbor.val[0]-scaPixel.val[0])/sigma,2)));
					neighborIdx		= areaWindow+yn*sizeWindow.width+xn;
					pGraph -> add_edge(nodes[nodeIdx], nodes[neighborIdx], weightEdge, 0);
				}
			}
		}
	}

	//compute Max Flow/Minimum Cut
	Graph::flowtype flow = pGraph -> maxflow();

	//get segment result
	double	sumx, sumy;
	int		fgCount;
	int				objWidth;
	int				objHeight;

	cvZero(gImg1FGMask);
	sumx = 0;
	sumy = 0;
	fgCount = 0;
	for(xi=0;xi<sizeWindow.width;xi++){
		for(yi=0;yi<sizeWindow.height;yi++){			 
			nodeIdx = areaWindow+yi*sizeWindow.width+xi;
			if (pGraph->what_segment(nodes[nodeIdx]) == Graph::SOURCE){				
				cvSetReal2D(gImg1FGMask, yi+rectWindow.top, xi+rectWindow.left, 255);
				sumx = sumx + xi;
				sumy = sumy + yi;
				fgCount++;
			}
		}
	}
	//gravity center
	objWidth			= inStartBox.right-inStartBox.left+1;
	objHeight			= inStartBox.bottom-inStartBox.top+1;

	if (!fgCount){
		//outResult->x0 = (int)((inStartBox.left+inStartBox.right)/2);
		//outResult->x0 = (int)((inStartBox.left+inStartBox.right)/2);
		outResult->targetBox = inStartBox;
	}
	else{
		outResult->x0 = rectWindow.left + (int)(sumx/fgCount);
		outResult->y0 = rectWindow.top + (int)(sumy/fgCount);
		outResult->targetBox.left	= max(0,outResult->x0-objWidth/2);
		outResult->targetBox.top	= max(0,outResult->y0-objHeight/2);
		outResult->targetBox.right	= min(inImage->width-1, outResult->targetBox.left + objWidth-1);
		outResult->targetBox.bottom	= min(inImage->height-1, outResult->targetBox.top + objHeight-1);
	}
	outResult->FGImage	= gImg1FGMask;
	outResult->FGMask	= gImg1FGMask;
	outResult->ObjMask	= gImg1FGMask;

	delete pGraph;

	//record current frame
	cvCopy(inImage, gImgPrevFrame, NULL);	
}

IplImage* graph_GetFGImage(){
	return gImg1FGMask;
}