// global data structures for mean shift tracking and image processing
// Bob Collins, Carnegie Mellon University, July 31, 2001


#define USECOLOR TRUE
#define MAXROWS 600
#define MAXCOLS 800
#define MAXBANDS 3
#define HALFROWS 300
#define HALFCOLS 400


#define MAXPIX (MAXROWS*MAXCOLS)
#define HALFPIX (HALFROWS*HALFCOLS)

#define REDPLANE_OFFSET 0
#define GREENPLANE_OFFSET (NGInumrows*NGInumcols)
#define BLUEPLANE_OFFSET (2*NGInumrows*NGInumcols)

//green plane
#define GRAY(im) (im+((NGInumbands==3) ? NGInumrows*NGInumcols : 0))

//typedef int bool;

extern char NGIfilepre[256], NGIfilepost[10];
extern int NGIfilestartframe, NGIfilecurframe, NGIfilenumdigits;
extern int NGInumrows, NGInumcols, NGInumbands;

extern float NGIbbox, NGIbbrow, NGIbbcol, NGIbbhrow, NGIbbhcol;
extern float NGIbbscale;

extern unsigned char NGIimage[MAXPIX*MAXBANDS];
extern unsigned char NGIrawinput[MAXPIX*MAXBANDS];
extern unsigned char NGIbhatim[MAXPIX];
extern unsigned char NGIindeximage[MAXPIX];
extern unsigned char NGItemp[MAXPIX*MAXBANDS];
extern float NGIftemp[MAXPIX];
extern int NGIitemp[MAXPIX];
extern unsigned char NGIsmooimage[HALFPIX];
extern short int NGIgradDX[HALFPIX], NGIgradDY[HALFPIX];


char *NGIfilename(int framenum);
