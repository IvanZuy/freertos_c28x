#ifndef __FPU_TEST_H__
#define __FPU_TEST_H__

/*
 * Calculating Pi - Machin Algorithm:
 * pi/4 = 4*arctan(1/5) - arctan(1/239)
 * Executing compute_pi_machin(1./5, 1./239) returns PI.
 */
double compute_pi_machin (double x, double y);

/*
 * Calculating Pi - Gauss Algorithm:
 * pi/4 = 12*arctan(1/18) + 8*arctan(1/57) - 5*arctan(1/239)
 * Executing compute_pi_gauss(1./18, 1./57, 1./239) returns PI.
 */
double compute_pi_gauss (double x, double y, double z);

/*
 * Calculating Pi - Brent-Salamin Algorithm:
 *  a[0] = 1
 *  b[0] = sqrt (2)
 *  t[0] = 1/4
 *  p[0] = 1
 *  a[n+1] = (a[n] + b[n]) / 2
 *  b[n+1] = sqrt (a[n] * b[n])
 *  t[n+1] = t[n] - p[n] * (a[n] - a[n+1])^2
 *  p[n+1] = 2 * p[n]
 * Result:
 *  pi = (a[n] + b[n])^2 / 4 * t[n]
 *
 * Executing compute_pi_brent(1.0, M_SQRT1_2l, 0.25, 1.0) returns PI
 */
double compute_pi_brent (double a0, double b0, double t0, double p0);

#endif // __FPU_TEST_H__
