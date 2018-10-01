// imageProcessing.cpp : image processing 

void separatePlanes(unsigned char *src, unsigned char *dest, int nr, int nc);

void mergePlanes(unsigned char *src, unsigned char *dest, int nr, int nc);

void crosshair(unsigned char *image, int nrows, int ncols, int numbands, int r, int c);

void dbox(unsigned char *im, int nrows, int ncols, int numbands, 
	  int row, int col, int hrow, int hcol);

void sobel_grad_filter(unsigned char *image, int xsize, int ysize, int deinterlace, 
					   unsigned char *smoo, short int *Dx, short int *Dy, int *temp);

void color_index_image (unsigned char *colorimage, int nrows, int ncols,
			unsigned char *indeximage, int reinitquant);

