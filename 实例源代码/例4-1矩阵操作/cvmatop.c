#include "cv.h"
#include "highgui.h"
#include <stdio.h>

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

int main()
{
//
// 测试矩阵操作
// 所有数据是按照MATLAB中的矩阵操作例子来进行的
//
    double a[] = {  1,  2,  0,
					2,  5, -1,
                    4, 10, -1 };
    double b[9],c[9];
    CvMat Ma, Mb, Mc;
    
    // 从缓存给矩阵赋值
    cvInitMatHeader( &Ma, 3, 3, CV_64FC1, a, CV_AUTOSTEP );
    cvInitMatHeader( &Mb, 3, 3, CV_64FC1, b, CV_AUTOSTEP );
    cvInitMatHeader( &Mc, 3, 3, CV_64FC1, c, CV_AUTOSTEP );

    // 显示矩阵Ma
    printf("A = "); PrintMat(&Ma);

    // 转置 Mb = Ma'
    cvTranspose(&Ma, &Mb);
    printf("A’ = "); PrintMat(&Mb);

    // Mc = Ma * Mb
    cvMatMul(&Ma, &Mb, &Mc);
    printf("A*A’ = "); PrintMat(&Mc);

    // Mc = Ma .* Mb
    cvMul(&Ma, &Mb, &Mc, 1);
    printf("A.*A = "); PrintMat(&Mc);

    // Mb = inv( Ma ), 用高斯消去法求逆
    cvInvert(&Ma, &Mb, CV_LU);
    printf("inv(A) = "); PrintMat(&Mb);

    // Ma * inv(Ma) 
    cvMatMul(&Ma, &Mb, &Mc);
    printf("A*inv(A) = "); PrintMat(&Mc);

    // Mc = svd(Ma),  表示 Ma = U * Mc * V'
    cvSVD(&Ma, &Mc, NULL, NULL, 0);  // 不返回 U 和 V
    printf("svd(A) = "); PrintMat(&Mc);

    // Mc = Ma + 2
    cvAddS(&Ma, cvRealScalar(2), &Mc, NULL);
    printf("A+2 = "); PrintMat(&Mc);

	return 0;
}
