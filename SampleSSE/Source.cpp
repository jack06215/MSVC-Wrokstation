#define ITERATION 100000
//#define USE_CPU
#define USE_SSE
//#define USE_DIVISION_METHOD
#define USE_FAST_METHOD

#include <xmmintrin.h>	// Need this for SSE compiler intrinsics
#include <math.h>		// Needed for sqrt() in CPU version
#include <stdio.h>		// Needed for printf() function
#include <time.h>		// Measure the execution time

int main(int argc, char* argv[])
{
	clock_t start_t = clock();
	printf("Starting calculation...\n");
	const int length = 64000;
	float *pResult = (float*)_aligned_malloc(length * sizeof(float), 16);	// align to 16-byte for SSE
	__m128 x;
	__m128 xDelta = _mm_set1_ps(4.0f);		// Set the xDelta to (4,4,4,4)
	__m128 *pResultSSE = (__m128*) pResult;
	const int SSELength = length / 4;
	for (int iter = ITERATION; iter--;)
	{
#ifdef USE_SSE
		x = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);	// Set the initial values of x to (4,3,2,1)
		for (int i = 0; i < SSELength; i++)
		{
			__m128 xSqrt = _mm_sqrt_ps(x);
#ifdef USE_FAST_METHOD
			__m128 xRecip = _mm_rcp_ps(x);
			pResultSSE[i] = _mm_mul_ps(xRecip, xSqrt);
#endif // !USE_FAST_METHOD
#ifdef USE_DIVISION_METHOD
			pResultSSE[i] = _mm_div_ps(xSqrt, x);
#endif	// !USE_DIVISION_METHOD
			x = _mm_add_ps(x, xDelta);	// Increment x to the next set of numbers
		}
#endif	// !USE_SSE
#ifdef USE_CPU
		float xFloat = 1.0f;
		for (int i = 0; i < length; i++)
		{
			pResult[i] = sqrt(xFloat) / xFloat;
			xFloat += 1.0f;
		}
#endif	// !USE_CPU
	}
	clock_t end_t = clock();
	printf("Execution time is %.3lf second(s)\n", ((double)(end_t - start_t) / CLOCKS_PER_SEC));
	for (int i = 0; i < 20; i++)
	{
		printf("Result[%d] = %f\n", i, pResult[i]);
	}
	return 0;
}