/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

#define MIN(x,y) ((x) < (y) ? (x) : (y))
int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */

char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    if (M == 32) {
// diagnoal is 1 miss A each row, 8 miss B first, 1 miss other -> 23/128
// non-diag is 1/8
// 1/4 * 23 / 128 + 1/8 * 3/4 = 0.168 -> 0.13867 * 2096 = 290
        for (int i = 0; i < 4; i++) {
	    for (int j = 0; j < 4; j++) {
		for (int k = (i << 3); k < (i << 3) + 8; k++) {
		    int m = (j << 3);
		    int a0 = A[k][m], a1 = A[k][m+1], a2 = A[k][m+2];
		    int a3 = A[k][m+3], a4 = A[k][m+4], a5 = A[k][m+5];
		    int a6 = A[k][m+6], a7 = A[k][m+7];
		    B[m][k] = a0; B[m+1][k] = a1; B[m+2][k] = a2;
		    B[m+3][k] = a3; B[m+4][k] = a4; B[m+5][k] = a5;
		    B[m+6][k] = a6; B[m+7][k] = a7;
		}
	    }
	} 
    } else if (M == 64) {
        for (int i = 0; i < 8; i++) {
	    for (int j = 0; j < 8; j++) {
		int m = (j << 3);
		int k, a0, a1, a2, a3, a4, a5, a6, a7;
		for (k = (i << 3); k < (i << 3) + 4; k++) {
		    a0 = A[k][m], a1 = A[k][m+1], a2 = A[k][m+2];
		    a3 = A[k][m+3], a4 = A[k][m+4], a5 = A[k][m+5];
		    a6 = A[k][m+6], a7 = A[k][m+7];
		    B[m][k] = a0; B[m+1][k] = a1; B[m+2][k] = a2;
		    B[m+3][k] = a3; B[m][k+4] = a4; B[m+1][k+4] = a5;
		    B[m+2][k+4] = a6; B[m+3][k+4] = a7; 
		}
		m = (i << 3) + 4;
		for (k = (j << 3); k < (j << 3) + 4; k++) {
		    a0 = A[m][k], a1 = A[m+1][k];
		    a2 = A[m+2][k], a3 = A[m+3][k];

		    a4 = B[k][m], a5 = B[k][m+1];
		    a6 = B[k][m+2], a7 = B[k][m+3];

		    B[k][m] = a0; B[k][m+1] = a1;
		    B[k][m+2] = a2; B[k][m+3] = a3;

		    B[k+4][m-4] = a4; B[k+4][m-3] = a5;
		    B[k+4][m-2] = a6; B[k+4][m-1] = a7;

		    B[k+4][m] = A[m][k+4]; B[k+4][m+1] = A[m+1][k+4];
		    B[k+4][m+2] = A[m+2][k+4]; B[k+4][m+3] = A[m+3][k+4];
		}
	    }
	}
    } else {
	int len = 23;
	int i_limit = (67 + len - 1)/len;
	int j_limit = (61 + len - 1)/len;
	for (int i = 0; i < i_limit; i++) {
	    for (int j = 0; j < j_limit; j++) {
		for (int k = i * len; k < MIN(N,i * len + len); k++) {
		    for (int m = j * len; m < MIN(M,j * len + len); m++) {
			B[m][k] = A[k][m];
		    }
		}
	    }
	}
    }   
}



/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }
}

char trans_desc2[] = "naive blocking scan transpose";
void trans2(int M, int N, int A[N][M], int B[M][N])
{
    if (M == 32) {
// diagnoal is 2 miss A each row, 8 miss B first, 2 miss other -> 38/128
// non-diag is 1/8
// 1/4 * 38 / 128 + 1/8 * 3/4 = 0.168 -> 0.168 * 2096 = 344
        for (int i = 0; i < 4; i++) {
	    for (int j = 0; j < 4; j++) {
		for (int k = (i << 3); k < (i << 3) + 8; k++) {
		    for (int m = (j << 3); m < (j << 3) + 8; m++) {
			B[m][k] = A[k][m];
		    }
		}
	    }
	} 
    } else if (M == 64) {
// diagnoal(4 * 4) is 1 miss A each row, 4 miss B first time, another 3 miss total for other each col -> (4 + 4 +3) / (16 * 2)
// non-diag is 1/8 for A each row, 1/4 for B each col -> 3/16 
// 11 / 32 * 1/8 + 3/16 * 7/8 = 0.168 -> 0.168 * 8192 = 1696
        for (int i = 0; i < 16; i++) {
	    for (int j = 0; j < 16; j++) {
		for (int k = (i << 2); k < (i << 2) + 4; k++) {
		    int m = (j << 2);
		    int a0 = A[k][m], a1 = A[k][m+1], a2 = A[k][m+2], a3 = A[k][m+3];
		    B[m][k] = a0; B[m+1][k] = a1; B[m+2][k] = a2; B[m+3][k] = a3;
		}
	    }
	}
    } else {
	for (int i = 0; i < 9; i++) {
	    for (int j = 0; j < 8; j++) {
		for (int k = (i << 3); k < MIN(N,(i << 3) + 8); k++) {
		    for (int m = (j << 3); m < MIN(M,(j << 3) + 8); m++) {
			B[m][k] = A[k][m];
		    }
		}
	    }
	} 
    }

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */

void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 
    registerTransFunction(trans2, trans_desc2);

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

