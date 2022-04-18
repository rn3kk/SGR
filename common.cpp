//---------------------------------------------------------------------------




#include <math.h>
#include <sstream>
#include "common.h"
#pragma hdrstop

const int SND_LEN = 12000;
const int SND_LEN2 = 48000;
const int SND_FD = 12000;  //12000
const int SND_FD2 = 48000;
const int DF_LEN = 1000; //усреднение частоты

//---------------------------------------------------------------------------

std::string getAvgMediumLevel(float * x, double &Usr)
{
  std::string res;
  float uu=0;
  for(int i=0; i<SND_LEN; i++)uu=uu+fabs(x[i]);
  uu=uu/SND_LEN;
  if(Usr == 0) Usr=uu;
  Usr=0.7*(Usr)+0.3*uu;

  if(Usr>0)
  {
    double v = 20*log10(Usr);
    std::ostringstream sstream;
    sstream << v;
    std::string str_v = sstream.str();
    res = "Volume " + str_v + " db";
  }
  return res;
};


void convertFrom48000_toDown(float* from, float* to, TIIRBPF2* filter1, TIIRBPF2* filter11, TIIRBPF2* filter111, double& dph2)
{
  const double twoPi = 2*M_PI;
  double ph2=M_PI/256.0;
  for(int i=0; i<SND_LEN2; i++)
  {
     to[i/4]=(filter111->filter(filter11->filter(filter1->filter(from[i])))) * cos(ph2);
     ph2=ph2+dph2; if(ph2>twoPi)ph2=ph2-twoPi;
  }
};

int nb(float *x, int nb_level)
{
  int cntClip=0;
  for(int i=0; i<SND_LEN; i++)
    if(fabs(x[i])>nb_level) {x[i]=0;cntClip++;}
  return cntClip;
};

static unsigned long CNT_Z=0; //счетчик переходов через ноль
static unsigned long CNT_Z_ALL=0;//cчетчик обработанных отсчетов
static int cnt_F=0;
int freqCounter(float *x, float* xf, float* xf2, double *df0, double &deltaF, double &d_deltaF, TIIRBPF2 *BPF_F, TIIRBPF2 *BPF_F1, TIIRBPF2 *BPF_F2)
{
  int Uf(0);
  int Uf2(0);
  int SNRF(0);

  for(int k0=0; k0<SND_LEN-SND_LEN/10; k0+=SND_LEN/10)
  {
    float uuf=0, uuf2=0;
    for(int i=0; i<SND_LEN/10; i++)
      uuf=uuf+fabs(xf[i+k0]);
    Uf=uuf;//0.7*Uf+0.3*(10*uuf/SND_LEN);
    for(int i=0; i<SND_LEN/10; i++)
      uuf2=uuf2+fabs(xf2[i+k0]);
    int Uf2=uuf2;//0.7*Uf2+0.3*(10*uuf2/SND_LEN);
    SNRF=0;
    if(Uf<Uf2)
      SNRF=20.0*log10(Uf/(Uf2-Uf));
    float ddf=0;
    if(SNRF > 6.0)
    {
         //счет переходов через ноль
      for(int i=0; i<SND_LEN/10; i++)
      {
        if(i+k0+1>=SND_LEN) continue;
        if(xf[i+k0]<0 && xf[i+k0+1]>=0)
          CNT_Z++;
        if(xf[i+k0]>=0 && xf[i+k0+1]<0)
          CNT_Z++;
        CNT_Z_ALL++;
      }
      if(1.0*CNT_Z_ALL/SND_FD>=2.0)
      {
        ddf=(1.0*CNT_Z/2.0/CNT_Z_ALL)-BPF_F->getF0();
        ddf=ddf*SND_FD;
        if(1.0*CNT_Z_ALL/SND_FD>=10.0)
        {
           CNT_Z_ALL=CNT_Z_ALL/2;
           CNT_Z=CNT_Z/2;
        }
        df0[cnt_F]=ddf;
        cnt_F++; cnt_F=cnt_F%DF_LEN;
        int cf=1;
        float df=0;
        for(int i=0; i<DF_LEN; i++)
        {
          if(df0[i]!=0 && fabs(df0[i])<20.0)
          {
            cf++;
            df+=df0[i];
          }
        }

        deltaF=df/cf;
        d_deltaF=0;
        for(int i=0; i<DF_LEN; i++)
           if(df0[i]!=0 && fabs(df0[i])<20.0)
             d_deltaF+=(df0[i]-deltaF)*(df0[i]-deltaF);

        d_deltaF=3.0*sqrt(d_deltaF)/cf;
        if(cf<10)
           d_deltaF=10.0;
      }
    }
  }       
  return 0;
};
#pragma package(smart_init)
