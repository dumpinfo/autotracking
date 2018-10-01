 #include <math.h>
#define NRANSI
#include "nrutil.h"

float pythag(float a, float b)
{
	float absa,absb;
	absa= (float)fabs(a);
	absb= (float)fabs(b);
	if (absa > absb) return absa*(float)sqrt(1.0+SQR(absb/absa));
	else return (float)(absb == 0.0 ? 0.0 : absb*sqrt(1.0+SQR(absa/absb)));
}
#undef NRANSI
