/* 程序名：cvmat.c
   功能：说明矩阵的一些操作方法
*/
#include "cv.h"
#include "highgui.h"
#include <stdio.h>

void PrintMat(CvMat *A);  // 显示矩阵
void GenRandn(CvMat *arr, int seed); // 生成正态分布的随机矩阵
void GenRand(CvMat *arr, int seed); // 生成[0，1]均匀分布的随机矩阵
static int cmp_func( const void* _a, const void* _b, void* userdata ); // 比较函数

void Test_Multiply();  // 测试矩阵乘法
void Test_cvGetRawData();  // 将缓存数据填入CvMat数组中
void Test_DCT();   // 计算DCT变换
void Test_Rand();  // 生成随机数
void Test_SeqSort();  // 二维序列排序

int main()
{
    Test_Multiply();        // pass
    Test_cvGetRawData();    // pass
    Test_DCT();             //pass
	Test_Rand();     // pass
	Test_SeqSort();  // pass
    return 0;
}

// Testing: Sort 2d points in top-to-bottom left-to-right order.
void Test_SeqSort()
{
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* seq = cvCreateSeq( CV_32SC2, sizeof(CvSeq), sizeof(CvPoint), storage );
	int i;
	
    printf("\n=== Test sequence sorting ===");

    for( i = 0; i < 10; i++ )
	{
		CvPoint pt;
		pt.x = rand() % 1000;  // 1000 以内的随机数
		pt.y = rand() % 1000;
		cvSeqPush( seq, &pt );
	}
	
	printf("\nOriginal point set:\n");
	for( i = 0; i < seq->total; i++ )
	{
		CvPoint* pt = (CvPoint*)cvGetSeqElem( seq, i );
		printf( "(%d,%d)\n", pt->x, pt->y );
	}

	cvSeqSort( seq, cmp_func, 0 /* userdata is not used here */ );
	
	/* print out the sorted sequence */
	printf("\nAfter sorting:\n");
	for( i = 0; i < seq->total; i++ )
	{
		CvPoint* pt = (CvPoint*)cvGetSeqElem( seq, i );
		printf( "(%d,%d)\n", pt->x, pt->y );
	}
	
	cvClearSeq( seq );   // Sequence clearing should be done before storage clearing
	cvReleaseMemStorage( &storage );
}

// The comparison function that returns negative, zero
// or positive value depending on the elements relation 
static int cmp_func( const void* _a, const void* _b, void* userdata )
{
    CvPoint* a = (CvPoint*)_a;
    CvPoint* b = (CvPoint*)_b;
    int y_diff = a->y - b->y;
    int x_diff = a->x - b->x;
    return y_diff ? y_diff : x_diff;
}

// 生成随机矩阵
void Test_Rand()
{
    CvMat* a = cvCreateMat( 10, 6, CV_32F );
    int i;
    printf("\n=== Test generating random matrix ===");
    for(i=0;i<5;i++)
    {
        GenRandn(a, i);
        PrintMat(a);
    }
    cvReleaseMat(&a);
}

// 显示矩阵
void PrintMat(CvMat* A)
{
    int i,j;
    //printf("\nMatrix = :");
    for(i=0;i<A->rows;i++)
    {
        printf("\n");
        
        switch( CV_MAT_DEPTH(A->type) )
        {
        case CV_32F:
        case CV_64F:
            for(j=0;j<A->cols;j++)
                printf("%9.3f ", (float) cvGetReal2D( A, i, j ));
            break;
        case CV_8U:
        case CV_16U:
            for(j=0;j<A->cols;j++)
                printf("%6d",(int)cvGetReal2D( A, i, j ));
            break;
        default:
            break;
        }
    }
    printf("\n");
}

/*
Generate uniformly distributed random numbers.
Fill random numbers to arr, chosen from 
a uniform distribution on the interval (0.0,1.0).
seed is the random generator seed. 
Array arr should be pre-allocated before calling this function
*/
void GenRand(CvMat* arr, int seed)
{
    // let's noisy_screen be the floating-point 2d array that is to be "crapped" 
    CvRandState rng;
    
    // initialize random generator
    rng.state = cvRNG(0xffffffff);
	cvRandInit( &rng,
        0, 1,      // use dummy parameters now and adjust them further 
        seed, // use input seed here 
        CV_RAND_UNI // specify uniform type  
        );
    
    cvRandArr( &rng.state, arr, CV_RAND_UNI, cvRealScalar(0), cvRealScalar(1) );
    // RNG state does not need to be deallocated 
}

/*
Generate normally distributed random numbers.
Fill random numbers to arr, of a normal distribution 
with mean zero and variance one.
seed is the random generator seed. 
Array arr should be pre-allocated before calling this function
*/
void GenRandn(CvMat* arr, int seed)
{
    // let's noisy_screen be the floating-point 2d array that is to be "crapped" 
    CvRandState rng;
    
    // modify RNG to make it produce normally distributed values
    rng.state = cvRNG(0xffffffff);
    cvRandInit( &rng,
        0, 1,      // use dummy parameters now and adjust them further 
        seed, // use input seed here 
        CV_RAND_NORMAL // specify uniform type  
        );
    // fill random numbers to arr, with mean zero and variance one  
    cvRandArr( &rng.state, arr, CV_RAND_NORMAL,
        cvRealScalar(0),  // average intensity
        cvRealScalar(1)   // deviation of the intensity
        );
    // RNG state does not need to be deallocated 
}

// Test matrix multiply
void Test_Multiply()
{
    double a[] = { 1,  2,  3,  4,
		5,  6,  7,  8,
				9, 10,  11, 12 };
    
    double b[] = { 	1, 5, 9,
					2, 6, 10,
					3, 7, 11,
					4, 8, 12 };
    
    double c[9];
    CvMat Ma, Mb, Mc;
    
    printf("\n=== Test multiply ===");
    cvInitMatHeader( &Ma, 3, 4, CV_64FC1, a, CV_AUTOSTEP );
    cvInitMatHeader( &Mb, 4, 3, CV_64FC1, b, CV_AUTOSTEP );
    cvInitMatHeader( &Mc, 3, 3, CV_64FC1, c, CV_AUTOSTEP );

    cvMatMulAdd( &Ma, &Mb, 0, &Mc );
    
    PrintMat(&Ma);
    PrintMat(&Mb);
    PrintMat(&Mc);
    return;
}

// Get raw data from data buffer and pass them to a matrix
void Test_cvGetRawData()
{
    float* data;
    int step;
    float a[] = {	 1, 2, 3, 4,
				 -5, 6, 7, 8,
				 9, -10, -11, 12 };
    CvMat array;
    CvSize size;
    int x, y;
    
    printf("\n=== Test get raw data ===");

    cvInitMatHeader( &array, 3, 4, CV_32FC1, a, CV_AUTOSTEP );
    
    cvGetRawData( &array, (uchar**)&data, &step, &size );
    step /= sizeof(data[0]);
    
    printf("\nCvMat = ");
    PrintMat(&array);
    printf("\nData = ");
    for( y = 0; y < size.height; y++, data += step )
    {
        printf("\n");
        for( x = 0; x < size.width; x++ )
        {
            data[x] = (float)fabs(data[x]);
            printf("%8.2f",data[x]);
        }
    }
    printf("\n");
    return;
}

// test 1-d and 2-d dct transform
void Test_DCT()
{
    float data[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    
    CvMat a;
    a = cvMat(2,4,CV_32FC1,data);

    printf("\n=== Test DCT ===");

    printf("\nOriginal matrix = "); PrintMat(&a);
    
    cvDCT(&a, &a, CV_DXT_FORWARD);
    printf("\n2-D DCT = "); PrintMat(&a);
	
    cvDCT(&a, &a, CV_DXT_INVERSE);
    printf("\n2-D IDCT = "); PrintMat(&a);
}
