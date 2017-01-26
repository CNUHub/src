#include <math.h>
#include <stdlib.h>
//#include <iostream.h>
#include <iostream>
#include "psytools.h"

/* Sort array a[] in place */
void shellsort(unsigned int n, double a[]) {
  // n = (input) size of arrays a[] and b[]
  // a[] = (input/output) array to sort in place
  unsigned int inc = 1;
  do {
    inc *= 3;
    inc ++;
  } while (inc <= n);
  do {
    inc /= 3;
    for( unsigned int i=inc; i<n; i++) {
      double v = a[i];
      unsigned int j = i;
      while (a[j - inc] > v) {
	a[j] = a[j-inc];
	j -= inc;
	if ((j+1) <= inc) break;
      }
      a[j] = v;
    }
  } while (inc > 1);
}

/* Sort array a[] in place with values in b[] also swapped to maintain
 same relative order */
void shellsort(unsigned int n, double a[], double b[]) {
  // n = (input) size of arrays a[] and b[]
  // a[] = (input/output) array to sort in place
  // b[] = (input/output) array to order in place with the same
  //                      re-ordering as a[]
  unsigned int inc = 1;
  do {
    inc *= 3;
    inc ++;
  } while (inc <= n);
  do {
    inc /= 3;
    for( unsigned int i=inc; i<n; i++) {
      double v = a[i];
      double w = b[i];
      unsigned int j = i;
      while (a[j - inc] > v) {
	a[j] = a[j-inc];
	b[j] = b[j-inc];
	j -= inc;
	if ((j+1) <= inc) break;
      }
      a[j] = v;
      b[j] = w;
    }
  } while (inc > 1);
}

/* Replace values in sorted array w[] with rank values */
void crank(unsigned int n, double w[], double *s) {
  // n = (input) size of array w[]
  // w[] = (input/output) array already sorted in increasing order whose
  //                      values are to be replaced with rank numbers
  // *s = (output) returns the sum of (f^3 - f) where f is the number
  //               of elements in each tie
  *s = 0;
  unsigned int j = 0;
  while (j < n-1) {
    if( w[j+1] != w[j] ) {
      w[j] = j+1;
      j++;
    }
    else {
      unsigned int jt = j + 1;
      for ( ; (jt < n) && (w[jt]==w[j]); jt++) ;
      double rank = 0.5 * (double) (j+1+jt);
      for ( unsigned int ji = j; ji<jt; ji++) w[ji] = rank;
      double t = jt - j;
      *s += t * t * t - t;
      j = jt;
    }
  }
  if ( j == (n-1) ) w[n-1] = n;
}
/*
  shellsort(n, wksp2, wksp1);
  double sg;
  crank(n, wksp2, &sg);
*/
/* Calculate the Spearman Rank-Order Correlation Coefficient for two
 arrays of ranked data */
double spear(unsigned int n,
	   double rank1[], double sf, double rank2[], double sg)
{
  // n = (input) size of arrays rank1[] and rank2[]
  // rank1[] = (input) array of rank values
  // sf = (input) sum of (f^3 - f) where f is the number of elements
  //      in each tie from rank1
  // rank2[] = (input) array of rank values
  // sg = (input) sum of (f^3 - f) where f is the number of elements
  //      in each tie from rank2
  // *d = (output) returns sum of square difference of rank
  // return = (output) returns linear correlation coefficient of the ranks
  unsigned int j = 0;
  double d = 0.0;
  for(j=0; j<n; j++) {
    double diff = rank1[j] - rank2[j];
    d += diff * diff;
  }
  double en = n;
  double en3n = en*en*en - en;
  double fac = (1.0 - sf/en3n) * (1.0 - sg/en3n);
  //Rank correlation coefficient
  double rs = 0;
  if(fac != 0) rs = (1.0 - (6.0/en3n) * (d + (sf + sg)/12.0))/sqrt(fac);
  return rs;
}
/* unused garbage
  double aved = en3n/6.0 - (sf + sg)/12.0;  // expectation value of D
  double vard = ((en-1.0)*en*en* (en+1.0)*(en+1.0)/36.0)*fac; // variance of D
  *zd = (*d-aved)/sqrt(vard);
  *probd = erfcc(fabs(*zd)/1.142136);
  fac = (rs + 1.0) * (1.0 - (rs));
  if(fac > 0.0) {
    double t = (rs) * sqrt((en - 2.0)/fac);
    double df = en - 2.0;
    *probrs = betai( 0.5 * df, 0.5, df/(df + t*t));
  }
  else *probrs + 0.0;
*/

/* Calculate natural log of gamma function */
double gammln(double xx)
{
  static double cof[6] = { 76.18009172947146, -86.50532032941677,
			   24.01409824083091, -1.231739572450155,
			   0.1208650973866179e-2, -0.5395239384953e-5 };
  double x = xx;
  double y = xx;
  double tmp = x + 5.5;
  tmp -= (x + 0.5) * log(tmp);
  double ser = 1.000000000190015;
  for (int j=0; j<6; j++) ser += cof[j]/++y;
  return -tmp + log(2.5066282746310005*ser/x);
}

#define EPS 3.0e-7
#define FPMIN 1e-30
#define MAXIT 100
/* Evaluates continued fraction for incomplete beta function */
double betacf( double a, double b, double x)
{
  double qab = a + b;
  double qap = a + 1.0;
  double qam = a - 1.0;
  double c = 1.0;
  double d = 1.0 - qab * x/qap;
  if( fabs(d) < FPMIN ) d = FPMIN;
  d = 1.0/d;
  double h = d;
  int m=1;
  for(; m<=MAXIT; m++) {
    int m2 = 2 * m;
    double aa = m * (b-m) * x/((qam + m2) * (a + m2));
    d = 1.0 + aa * d;

    if( fabs(d) < FPMIN ) d = FPMIN;
    d = 1.0/d;
    if( fabs(c) < FPMIN ) c = FPMIN;
    c = 1.0 + aa/c;

    h *= d * c;
    aa = -(a + m) * (qab + m) * x/((a + m2) * (qap + m2));
    d = 1.0 + aa * d;

    if( fabs(d) < FPMIN ) d = FPMIN;
    d = 1.0/d;
    if( fabs(c) < FPMIN ) c = FPMIN;
    c = 1.0 + aa/c;

    double del = d * c;
    h *= del;
    if ( fabs(del - 1.0) < EPS) break;
  }
  if(m > MAXIT) {
    std::cerr<<"psytools.cc:betacf error - a or b too big or MAXIT too small\n";
    exit(1);
  }
  return h;
}

/* Calculate the incomplete Beta Function Ix(a,b) */
double betai(double a, double b, double x)
{
  double bt;
  if(x < 0.0 || x > 1.0) {
    std::cerr<<"psytools.cc:betai error - bad x\n";
    return -1; // error
  }
  else if( x == 0.0 ) return 0;
  else if( x == 1.0 ) return 1.0;
  bt = exp(gammln(a + b) - gammln(a) - gammln(b) +
	   a * log(x) + b * log(1.0-x) );
  if( x < (a+1.0)/(a + b + 2.0) ) {
    return bt * betacf(a, b, x)/a;
  }
  else {
    return 1.0 - bt * betacf(b, a, 1 - x)/b;
  }
}
