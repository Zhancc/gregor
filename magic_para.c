/* 
 * Rectangular matrix multiplication.
 *
 * See the paper ``Cache-Oblivious Algorithms'', by
 * Matteo Frigo, Charles E. Leiserson, Harald Prokop, and 
 * Sridhar Ramachandran, FOCS 1999, for an explanation of
 * why this algorithm is good for caches.
 *
 * Author: Matteo Frigo
 */
static const char *ident __attribute__((__unused__))
     = "$HeadURL: https://bradley.csail.mit.edu/svn/repos/cilk/5.4.3/examples/matmul.cilk $ $LastChangedBy: sukhaj $ $Rev: 517 $ $Date: 2003-10-27 10:05:37 -0500 (Mon, 27 Oct 2003) $";

/*
 * Copyright (c) 2003 Massachusetts Institute of Technology
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "gregor.c"
#include "thread.c"
#include "getoptions.c"

#define REAL float
int call_time = 0;

extern int Cilk_rand(void);

void zero(REAL *A, int n)
{
     int i, j;
     
     for (i = 0; i < n; i++) {
      for (j = 0; j < n; j++) {
           A[i * n + j] = 0.0;
      }
     }
}

void initz(REAL *A, int n)
{
     int i, j;
     
     srand(time(NULL));
     for (i = 0; i < n; i++) {
      for (j = 0; j < n; j++) {
           A[i * n + j] = rand();
      }
     }
}

double maxerror(REAL *A, REAL *B, int n)
{
     int i, j;
     double error = 0.0;
     
     for (i = 0; i < n; i++) {
      for (j = 0; j < n; j++) {
           double diff = (A[i * n + j] - B[i * n + j]) / A[i * n + j];
           if (diff < 0)
            diff = -diff;
           if (diff > error)
            error = diff;
      }
     }
     return error;
}

void iter_matmul(REAL *A, REAL *B, REAL *C, int n)
{
     int i, j, k;
     
     for (i = 0; i < n; i++)
      for (k = 0; k < n; k++) {
           REAL c = 0.0;
           for (j = 0; j < n; j++)
            c += A[i * n + j] * B[j * n + k];
           C[i * n + k] = c;
      }
}

/*
 * A \in M(m, n)
 * B \in M(n, p)
 * C \in M(m, p)
 */
void rec_matmul(REAL *A, REAL *B, REAL *C, int m, int n, int p, int ld,
             int add)
{
     ++call_time;
     if ((m + n + p) <= 64) {
      int i, j, k;
      /* base case */
      if (add) {
           for (i = 0; i < m; i++)
            for (k = 0; k < p; k++) {
             REAL c = 0.0;
             for (j = 0; j < n; j++)
                  c += A[i * ld + j] * B[j * ld + k];
             C[i * ld + k] += c;
            }
      } else {
           for (i = 0; i < m; i++)
            for (k = 0; k < p; k++) {
             REAL c = 0.0;
             for (j = 0; j < n; j++)
                  c += A[i * ld + j] * B[j * ld + k];
             C[i * ld + k] = c;
            }
      }
     } else if (m >= n && n >= p) {
      int m1 = m >> 1;
      spawn(VOID, NULL, rec_matmul, 8, PTR, PTR, PTR, INT, INT, INT, INT, INT, A, B, C, m1, n, p, ld, add);
      spawn(VOID, NULL, rec_matmul, 8, PTR, PTR, PTR, INT, INT, INT, INT, INT, A + m1 * ld, B, C + m1 * ld, m - m1, n, p, ld, add);

      // rec_matmul(A, B, C, m1, n, p, ld, add);
      // rec_matmul(A + m1 * ld, B, C + m1 * ld, m - m1, n, p, ld, add);
      // spawn rec_matmul(A, B, C, m1, n, p, ld, add);
      // spawn rec_matmul(A + m1 * ld, B, C + m1 * ld, m - m1, n, p, ld, add);
     } else if (n >= m && n >= p) {
      int n1 = n >> 1;

      spawn(VOID, NULL, rec_matmul, 8,PTR, PTR, PTR, INT, INT, INT, INT, INT, A, B, C, m, n1, p, ld, add);      
      // rec_matmul(A, B, C, m, n1, p, ld, add);
      // spawn rec_matmul(A, B, C, m, n1, p, ld, add);
      __gregor_sync();
      spawn(VOID, NULL, rec_matmul, 8,PTR, PTR, PTR, INT, INT, INT, INT, INT, A + n1, B + n1 * ld, C, m, n - n1, p, ld, 1);      

      // rec_matmul(A + n1, B + n1 * ld, C, m, n - n1, p, ld, 1);
      // spawn rec_matmul(A + n1, B + n1 * ld, C, m, n - n1, p, ld, 1);
     } else {
      int p1 = p >> 1;
      spawn(VOID, NULL, rec_matmul, 8,PTR, PTR, PTR, INT, INT, INT, INT, INT, A, B, C, m, n, p1, ld, add);
      spawn(VOID, NULL, rec_matmul, 8,PTR, PTR, PTR, INT, INT, INT, INT, INT, A, B + p1, C + p1, m, n, p - p1, ld, add);

      // rec_matmul(A, B, C, m, n, p1, ld, add);
      // rec_matmul(A, B + p1, C + p1, m, n, p - p1, ld, add);
      // spawn rec_matmul(A, B, C, m, n, p1, ld, add);
      // spawn rec_matmul(A, B + p1, C + p1, m, n, p - p1, ld, add);
     }
}

int g_main(int argc, char *argv[])
{
     clock_t start_t, end_t;
 //    start_t = clock();
     int n;
     REAL *A, *B, *C1, *C2;
     double err;
     // Cilk_time tm_begin, tm_elapsed;
     // Cilk_time wk_begin, wk_elapsed;
     // Cilk_time cp_begin, cp_elapsed;

     if (argc != 2) {
      fprintf(stderr, "Usage: matmul [<cilk options>] <n>\n");
      // Cilk_exit(1);
      exit(1);
     }
     n = atoi(argv[1]);

     A = malloc(n * n * sizeof(REAL));
     B = malloc(n * n * sizeof(REAL));
     C1 = malloc(n * n * sizeof(REAL));
     C2 = malloc(n * n * sizeof(REAL));
      
     initz(A, n);
     initz(B, n);
     zero(C1, n);
     zero(C2, n);

     iter_matmul(A, B, C1, n);

     /* Timing. "Start" timers */
     // sync;
     // cp_begin = Cilk_user_critical_path;
     // wk_begin = Cilk_user_work;
     // tm_begin = Cilk_get_wall_time();

     // rec_matmul(A, B, C2, n, n, n, n, 0); 
     spawn(VOID, NULL, rec_matmul,8, PTR, PTR, PTR, INT, INT, INT, INT, INT, A, B, C2, n, n, n, n, 0);
     __gregor_sync();

     /* Timing. "Stop" timers */
     // tm_elapsed = Cilk_get_wall_time() - tm_begin;
     // wk_elapsed = Cilk_user_work - wk_begin;
     // cp_elapsed = Cilk_user_critical_path - cp_begin;

     err = maxerror(C1, C2, n);

     // printf("\nCilk Example: matmul\n");
     // printf("         running on %d processor%s\n\n",
        // Cilk_active_size, Cilk_active_size > 1 ? "s" : "");
     // printf("Max error     = %g\n", err);
     // printf("Options: size = %d\n", n);
     // printf("Running time  = %4f s\n", Cilk_wall_time_to_sec(tm_elapsed));
     // printf("Work          = %4f s\n", Cilk_time_to_sec(wk_elapsed));
     // printf("Critical path = %4f s\n", Cilk_time_to_sec(cp_elapsed));
     // printf("``MFLOPS''    = %4f\n\n",
        // 2.0 * n * n * n / (1.0e6 * Cilk_wall_time_to_sec(tm_elapsed)));

     free(C2);
     free(C1);
     free(B);
     free(A);
     printf("[SPAWN TIME] %d\n", call_time);
   //  end_t = clock();
   //  printf("Total time taken by CPU: %d\n", end_t - start_t);
     return 0;
}

int main(int argc, char *argv[]) {
  clock_t start_t,end_t;
  struct timespec start, finish;
  double elapsed;

  clock_gettime(CLOCK_MONOTONIC, &start);
  // start_t = clock();
  gregor_main(NULL, g_main, argc, argv);
  // end_t = clock();
  clock_gettime(CLOCK_MONOTONIC, &finish);

  elapsed = (finish.tv_sec - start.tv_sec);
  elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
  printf("Total time %f\n", elapsed);
}


