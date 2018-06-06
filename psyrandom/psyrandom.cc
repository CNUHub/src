#include "psyhdr.h"
#include <stdio.h>
#include <float.h>

#ifdef RANDOM_UNDEFINED
extern "C" {
  long random();
  int srandom(int seed);
  char *initstate(unsigned seed, char *state, int n);
  char *setstate(char *state);
}
#endif

const long IM1 = 2147483563;
const long IM2 = 2147483399;
const double AM = (1.0/IM1);
const long IMM1 = (IM1 - 1);
const int IA1 = 40014;
const int IA2 = 40692;
const int IQ1 = 53668;
const int IQ2 = 52774;
const int IR1 = 12211;
const int IR2 = 3791;
const int NTAB = 32;
const double NDIV = (1 + IMM1 / NTAB);
//const double RNMX (1.0 - MINDOUBLE);
const double RNMX (1.0 - DBL_MIN);

double ran2(long *idum) {
  // basic algorithm based on ran2 from "Numerical Recipes in C"
  // implementing the generator of L'Ecuyer with Bays-Durham shuffle
  int j;
  long k;
  static long idum2 = 123456789;
  static long iy = 0;
  static long iv[NTAB];
  double temp;

  if(*idum <= 0) {
    if( -(*idum) < 1) *idum = 1;
    else *idum = -(*idum);
    idum2 = (*idum);
    for( j = NTAB+7; j >= 0; j--) {
      k = (*idum) / IQ1;
      *idum = (IA1 * (*idum - (k * IQ1))) - (k * IR1);
      if (*idum < 0) *idum += IM1;
      if(j < NTAB) iv[j] = *idum;
    }
    iy = iv[0];
  }
  k = (*idum) / IQ1;
  *idum = (IA1 * (*idum - (k * IQ1))) - (k * IR1);
  if (*idum < 0) *idum += IM1;
  k = (idum2) / IQ1;
  idum2 = (IA2 * (idum2 - (k * IQ2))) - (k * IR2);
  if (idum2 < 0) idum2 += IM2;
  j = (int) (((double) iy)/NDIV);
  iy = iv[j] - idum2;
  iv[j] = *idum;
  if(iy < 1) iy += IMM1;
  if( (temp = AM * iy) > RNMX) return RNMX;
  else return temp;
}

double gassdev(long *idum) {
  static int iset = 0;
  static double gset;
  double fac, rsq, v1, v2;
  if (iset == 0) {
    do {
      v1 = (2.0 * ran2(idum)) - 1.0;
      v2 = (2.0 * ran2(idum)) - 1.0;
      rsq = (v1*v1) + (v2*v2);
    } while ((rsq >= 1.0) || (rsq == 0.0));
    fac = sqrt( (-2.0 * log(rsq)) / rsq );
    gset = v1 * fac;
    iset = 1;
    return v2 * fac;
  }
  else {
    iset = 0;
    return gset;
  }
}

enum algorithms { RANDOM, RAN2 };

int main(int argc, char *argv[])
{
  static long state1[32] = {
    3,
    0x9a319039, 0x32d9c024, 0x9b663182, 0x5da1f342,
    0x7449e56b, 0xbeb1dbb0, 0xab5c5918, 0x946554fd,
    0x8c2e680f, 0xeb3d799f, 0xb11ee0b7, 0x2d436b86,
    0xda672e2a, 0x1588ca88, 0xe369735d, 0x904f35f7,
    0xd7158fd6, 0x6fa6f051, 0x616e6b96, 0xac94efdc,
    0xde3b81e0, 0xdf0a6fb5, 0xf103bc02, 0x48f340fb,
    0x36413f93, 0xc622c298, 0xf5a42ab8, 0x8a88d77b,
    0xf5ad9d0e, 0x8999220b, 0x27fb47b9
  };
  int n=128;
//  unsigned seed=1;
  long seed=1;
  int number=1;
  double min=0.0;
  double max=1.0;
  int algorithm = RANDOM;
  int gauss = 0;
// parse input parameters
  int i;
  for(i=0; i<argc; i++) {
    if(strcmp(argv[i], "-help") == 0 || argc<1) {
      cout <<"Usage: "<<argv[0]<<" [-s iseed] [-n number]\n";
      cout <<"        [-r min{mean},max{stddev}] [-a2] [-gauss]\n";
      exit(0);
    }
    else if(i==0); // ignore program name
    else if(strcmp(argv[i],"-a2")==0) algorithm = RAN2;
    else if(strcmp(argv[i],"-gauss")==0) gauss = 1;
    else if((strcmp(argv[i],"-s")==0)&&((i+1)<argc)) {
      if(sscanf(argv[++i],"%ld", &seed) != 1) {
	cerr << argv[0] << ": error parsing seed: ";
	cerr << argv[i] << '\n';
	exit(1);
      }
    }
    else if((strcmp(argv[i],"-n")==0)&&((i+1)<argc)) {
      if(sscanf(argv[++i],"%d", &number) != 1) {
	cerr << argv[0] << ": error parsing number: ";
	cerr << argv[i] << '\n';
	exit(1);
      }
    }
    else if((strcmp(argv[i],"-r")==0)&&((i+1)<argc)) {
      if(sscanf(argv[++i],"%lf,%lf", &min, &max) != 2) {
	cerr << argv[0] << ": error parsing range: ";
	cerr << argv[i] << '\n';
	exit(1);
      }
    }
    else {
      cerr << argv[0] << ": unknown parameter: " << argv[i] << '\n';
      exit(1);
    }
  }

  if(gauss == 1) {
    double mean = min;
    double stddev = max;
    if(seed > 0) seed = -seed;
    for( i=0; i<number; i++) {
      printf("%lf\n", (gassdev(&seed) * stddev) + mean);
    }
  }
  else if(algorithm == RAN2) {
    if(seed > 0) seed = -seed;
    double factor = max - min;
    for( i=0; i<number; i++) {
      printf("%lf\n", (ran2(&seed)*factor)+min);
    }
  }
  else {
    initstate(seed, (char *) state1, n);
    setstate((char *)state1);
    //  long denom=2147483646; // 2^31-1;
    double factor = (max-min)/(double)2147483646;
    for( i=0; i<number; i++) {
      printf("%lf\n", (((double)random())*factor)+min);
    }
  }
}
