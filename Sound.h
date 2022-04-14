//---------------------------------------------------------------------------

#ifndef SoundH
#define SoundH

#include <mmsystem.h>
#include <math.h>
#include <mem.h>
#include <values.h>
#include <limits.h>
/////// определения для работы со звуковой картой ///////////////////////////
#define NOUTBUFS        4
#define NINBUFS         16
#define TWOPI           2*M_PI
#define SIZE_TABLE      16384
#define N_LOOP_MAX      INT_MAX
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/////////////////// РАБОТА СО ЗВУКОВОЙ КАРТОЙ //////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class TSound
{ protected:
  private:
        bool IsInOpen, IsOutOpen;
        bool IsOutPaused, IsOutReset;
        WAVEFORMATEX waveOutFormat,waveInFormat;
        HWAVEOUT outHandle;
        HWAVEIN inHandle;
        WAVEHDR waveOutHeader[NOUTBUFS];
        WAVEHDR waveInHeader[NINBUFS];
        unsigned char * InStor;
        unsigned char * OutStor;
        int InBufSize, OutBufSize;
        int curentInBuf, curentOutBuf;
        int posInBuf, posOutBuf;
        DWORD VolumeOut;
        
  public:
        int InBufCnt;
        int InDataReady[NINBUFS];
        bool OutDataEmpty[NOUTBUFS];
        bool InOverflow, OutUnderflow;
        CRITICAL_SECTION SoundInCriticalSection;
        CRITICAL_SECTION SoundOutCriticalSection;
        HANDLE InEvent, OutEvent;
        UINT IN_DEVICE, OUT_DEVICE;
        
        TSound();
        ~TSound();
        bool openIn(int samples_per_sec, int bit_per_sample, int channels, int samples_buf_size);
        bool openOut(int samples_per_sec, int bit_per_sample, int channels, int samples_buf_size);
        void closeIn(void);
        void closeOut(void);
        int read(float * left_samples, float * right_samples, int n_samples);
        int write(float * left_samples, float * right_samples, int n_samples);
        bool openOutLoop(int samples_per_sec, int bit_per_sample, int channels, int samples_buf_size, bool isLoop, int n_loop_blocks);
        bool pauseOut(void);
        bool restartOut(void);
        bool resetOut(void);

};
 void CALLBACK WaveOutProc(HWAVEOUT outHandle, UINT uMsg, TSound *snd,
                           DWORD dwParam1, DWORD dwParam2);
 void CALLBACK WaveInProc(HWAVEIN inHandle, UINT uMsg, TSound *snd,
                           DWORD dwParam1, DWORD dwParam2);
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


//---------------------------------------------------------------------------

#endif
