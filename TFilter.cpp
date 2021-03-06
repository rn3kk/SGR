#include <math.h>
#include <values.h>
#include <mem.h>
#include "TFilter.h"

#ifndef TWOPI
#define TWOPI 2*M_PI
#endif

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
TFilter::TFilter(float F1, float F2)
{ /* ??????????? ?????????? ???????
     ??????? ?????????????
     F1 - ?????? ??????? ?????
     F2 - ??????? ??????? ?????
  */
   F0=(F1+F2)/2;
   N= fabs(3.0*M_PI/(F2-F1));
   if(N%2)N=N+1;
   if(N>256)N=256;
   Z=new float[N+1];
   B=new float[N+1];
   memset(Z,0.0,N*sizeof(float)); memset(B,0.0,N*sizeof(float));

   // ?????? ????-???
   float G=0.0;
   for(int n=0; n<N; n++)
   {
    float t=n-N/2;
    B[n]=0.42+0.5*cos(TWOPI*t/N)+0.08*cos(2*TWOPI*t/N);   // ????????-???????
    B[n]=B[n]*sin(TWOPI*F0*n);
    G+=fabs(B[n]);
   }
   for(int n=0; n<N; n++) B[n]=B[n]/G;
}
//------------------------------------------------------------------------------
TFilter::~TFilter()
{
  delete[] Z; delete[] B;
}
//------------------------------------------------------------------------------
void TFilter::filter(float * x, int n, float * y)
{ /* ????????? ???????
     x[] - ??????? ???????
     n   - ?????????? ????????
     y[] - ???????? ???????
  */
  for(int i=0; i<n; i++)
  {
    memmove(&Z[1],Z,(N-1)*sizeof(float));
    Z[0]=x[i];
    float sum=0.0;
    for(int k=0; k<N; k++) sum+=Z[k]*B[k];
    y[i]=sum;
  }
}
//------------------------------------------------------------------------------
float TFilter::filter(float x)
{ /* ????????? ???????
     x[] - ??????? ???????
     n   - ?????????? ????????
     y[] - ???????? ???????
  */
    memmove(&Z[1],Z,(N-1)*sizeof(float));
    Z[0]=x;
    float sum=0.0;
    for(int k=0; k<N; k++) sum+=Z[k]*B[k];
    return sum;
}
//------------------------------------------------------------------------------
float TFilter::getF0(void)
{
     return (F0);
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
