#include <math.h>
#include <values.h>
#include <mem.h>
#include "TIIRLPF.h"

#ifndef TWOPI
#define TWOPI 2*M_PI
#endif

TIIRLPF::TIIRLPF(float Fs, int Nz)
{ /*
     Fs - частота среза фильтра
     Nz - кол-во звеньев
   */
   double alfa=exp(-2*M_PI*Fs);
   A=1.0-alfa;
   B=alfa;
   N=Nz;
   Y=new double[N];
   memset(Y,0,N*sizeof(double));
}
//------------------------------------------------------------------------------
TIIRLPF::~TIIRLPF()
{
  delete[] Y;
}
//------------------------------------------------------------------------------
double TIIRLPF::filter(double x)
{
   double y=0;
   Y[0]=A*x+B*Y[0];
   for(int i=1; i<N;i++) Y[i]=A*Y[i-1]+B*Y[i];
   y=Y[N-1];
   return y;
}
//------------------------------------------------------------------------------
float TIIRLPF::filter(float * x, int n)
{
   double Gain=0;
   for (int i=0; i<n; i++)
   {
      x[i]=filter(x[i]);
   }
   return Gain;
}
//------------------------------------------------------------------------------
 