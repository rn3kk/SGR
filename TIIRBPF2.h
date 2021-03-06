/* ??????????? ????????? ?????? 2-?? ??????? */

#ifndef TIIRBPF2H
#define TIIRBPF2H

class TIIRBPF2
{
public:
  double f0, ph, dph, ddph;
  TIIRBPF2(float Fc, float BW);
  float filter(float x);
  float filter(float x[], int n, float y[]);
  float getF0(void);
  float getBW(void);

private:
  double A0, A1, A2;
  double B1, B2;
  double Y0, Y1, Y2;
  double X0, X1, X2;
  float F0, BW0;
};

#endif
