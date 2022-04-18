//---------------------------------------------------------------------------

#ifndef commonH
#define commonH

#include <string>
#include "TIIRBPF2.h"


//---------------------------------------------------------------------------
std::string getAvgMediumLevel(float *x, double &Usr);
void convertFrom48000_toDown(float* from, float* to, TIIRBPF2* filter1, TIIRBPF2* filter11, TIIRBPF2* filter111, double& dph2);
int nb(float *x, int nb_level);
int freqCounter(float *x, float* xf, float* xf2, double *df0, double &deltaF, double &d_deltaF, TIIRBPF2 *BPF_F, TIIRBPF2 *BPF_F1, TIIRBPF2 *BPF_F2);
#endif
