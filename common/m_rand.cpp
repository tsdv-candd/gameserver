#include "m_rand.h"

#define NTAB 32
#define NWUP 8
#define NDIV (1+(IM-1)/NTAB)
#define EPS 1.2e-7
#define RNMX (1.0-EPS)

/* Minimal portable random generator by Park and Miller */

/* Lewis-Goodman-Miller constants */
#define IA 16807
#define IM 2147483647
#define AM (1./IM)
/* Scharge constants */
#define IQ 12773
#define IR 2836
/* Special mask to be explained below */
#define MASK 123456789

static long dummy;

/* initial seed, for all the generators here */
void Seed(long dum) {dummy=dum;}

/* returns random uniformly distributed between 0 and 1 */
float unirand0(void) {
 long k;
 float ans;
 dummy^=MASK;	/* avoid dummy==0 */
 k=dummy/IQ;
 if((dummy=IA*(dummy-k*IQ)-IR*k)<0) dummy+=IM;
 ans=(float)(AM*dummy);
 dummy^=MASK;	/* restore unmasked dummy */
 return(ans);
}




namespace m_rand_1 {

/* Minimal random generator with Bays-Durham shuffle */

/* previous functions, variables and constants are described above.
   Only additional follow */

float Rand(void)
{
 int j;
 long k;
 static long iy=0,iv[NTAB];
 float temp;
 /* initialize */
 if(dummy<=0 || !iy) {
  /* avoid negative or zero seed */
  if(dummy<0) dummy=-dummy;
  else if(dummy==0) dummy=1;
  /* after NWUP warmups, initialize shuffle table */
  for(j=NTAB+NWUP-1;j>=0;j--) {
   k=dummy/IQ;
   if((dummy=IA*(dummy-k*IQ)-IR*k)<0) dummy+=IM;
   if(j<NTAB) iv[j]=dummy;
  }
  /* first specimen from the table */
  iy=iv[0];
 }
 /* regular work: generate new number */
 k=dummy/IQ;
 if((dummy=IA*(dummy-k*IQ)-IR*k)<0) dummy+=IM;
 /* shuffle output */
 iy=iv[j=iy/NDIV];iv[j]=dummy;
 /* return */
 if((temp=(float)(AM*iy))>RNMX) return(float)(RNMX);
 else return(temp);
}

}





namespace m_rand_2 {

/* L'Ecuyer algorithm for uniform random generator
  with practically endless period. Combining 2 sequences. */

/* previous constants, static variables and functions are valid as if 
   the programs on this page are determined in one module */
#define IM1 2147483563
#define IM2 2147483399
#undef AM
#define AM (1./IM1)
#define IMM1 (IM1-1)
#define IA1 40014
#define IA2 40692
#define IQ1 53668
#define IQ2 52774
#define IR1 12211
#define IR2 3791
#undef NDIV
#define NDIV (1+IMM1/NTAB)

float Rand(void) {
 int j;
 long k;
 static long dummy2=123456789;
 static long iy=0;
 static long iv[NTAB];
 float temp;
 /* initialize the random sequence (first set of coefficients, the
    routine close to that in the function above */
 if(dummy<=0 || !iy) {
  /* avoid negative or zero seed */
  if(dummy<0) dummy=-dummy;
  else if(dummy==0) dummy=1;
  dummy2=dummy;
  /* after NWUP warmups, initialize shuffle table */
  for(j=NTAB+NWUP-1;j>=0;j--) {
   k=dummy/IQ1;
   if((dummy=IA1*(dummy-k*IQ1)-IR1*k)<0) dummy+=IM1;
   if(j<NTAB) iv[j]=dummy;
  }
  /* first specimen from the table */
  iy=iv[0];
 }
 /* regular work: generate 2 sequences. */
 k=dummy/IQ1;
 if((dummy=IA1*(dummy-k*IQ1)-IR1*k)<0) dummy+=IM1;
 k=dummy2/IQ2;
 if((dummy2=IA2*(dummy2-k*IQ2)-IR2*k)<0) dummy2+=IM2;
 /* shuffle output combining 2 sequences */
 iy=iv[j=iy/NDIV]-dummy2;iv[j]=dummy;
 if(iy<1) iy+=IMM1;
 /* return the result, as in the previous function */
 if((temp=(float)(AM*iy))>RNMX) return(float)(RNMX);
 else return(temp);
}

}
