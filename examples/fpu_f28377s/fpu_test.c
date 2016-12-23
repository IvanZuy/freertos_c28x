#include <math.h>
#include "fpu_test.h"

//-------------------------------------------------------------------------------------------------
double compute_pi_machin (double x, double y)
{
  double x2 = x * x;
  double x3 = x2 * x;
  double x5 = x3 * x2;
  double x7 = x5 * x2;
  double x9 = x7 * x2;
  double x11 = x9 * x2;
  double x13 = x11 * x2;
  double x15 = x13 * x2;
  double x17 = x15 * x2;
  double x19 = x17 * x2;
  double atx = x17/17 - x19/19;
  atx += x13/13 - x15/15;
  atx += x9/9 - x11/11;
  atx += x5/5 - x7/7;
  atx += x - x3/3;

  double y2 = y * y;
  double y3 = y2 * y;
  double y5 = y3 * y2;
  double y7 = y5 * y2;
  double y9 = y7 * y2;
  double y11 = y9 * y2;
  double y13 = y11 * y2;
  double y15 = y13 * y2;
  double aty = y13/13 - y15/15;
  aty += y9/9 - y11/11;
  aty += y5/5 - y7/7;
  aty += y - y3/3;
  return 4 * (4*atx - aty);
}

//-------------------------------------------------------------------------------------------------
double compute_pi_gauss (double x, double y, double z)
{
  double x2 = x * x;
  double x3 = x2 * x;
  double x5 = x3 * x2;
  double x7 = x5 * x2;
  double x9 = x7 * x2;
  double x11 = x9 * x2;
  double x13 = x11 * x2;
  double x15 = x13 * x2;
  double atx = x13/13 - x15/15;
  atx += x9/9 - x11/11;
  atx += x5/5 - x7/7;
  atx += x - x3/3;

  double y2 = y * y;
  double y3 = y2 * y;
  double y5 = y3 * y2;
  double y7 = y5 * y2;
  double y9 = y7 * y2;
  double y11 = y9 * y2;
  double y13 = y11 * y2;
  double y15 = y13 * y2;
  double aty = y13/13 - y15/15;
  aty += y9/9 - y11/11;
  aty += y5/5 - y7/7;
  aty += y - y3/3;

  double z2 = z * z;
  double z3 = z2 * z;
  double z5 = z3 * z2;
  double z7 = z5 * z2;
  double z9 = z7 * z2;
  double z11 = z9 * z2;
  double z13 = z11 * z2;
  double z15 = z13 * z2;
  double atz = z13/13 - z15/15;
  atz += z9/9 - z11/11;
  atz += z5/5 - z7/7;
  atz += z - z3/3;
  return 4 * (12*atx + 8*aty - 5*atz);
}

//-------------------------------------------------------------------------------------------------
double compute_pi_brent (double a0, double b0, double t0, double p0)
{
  double a1 = (a0 + b0) / 2;
  double b1 = sqrt (a0 * b0);
  double t1 = t0 - p0 * (a0 - a1)*(a0 - a1);
  double p1 = p0 + p0;

  double a2 = (a1 + b1) / 2;
  double b2 = sqrt (a1 * b1);
  double t2 = t1 - p1 * (a1 - a2)*(a1 - a2);
  double p2 = p1 + p1;

  double a3 = (a2 + b2) / 2;
  double b3 = sqrt (a2 * b2);
  double t3 = t2 - p2 * (a2 - a3)*(a2 - a3);
  return (a3 + b3)*(a3 + b3) / (4 * t3);
}
