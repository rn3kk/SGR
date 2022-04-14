//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Sound.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/////////////////// РАБОТА СО ЗВУКОВОЙ КАРТОЙ// ////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
TSound::TSound()
{
  InitializeCriticalSection(&SoundInCriticalSection);
  InitializeCriticalSection(&SoundOutCriticalSection);
  InEvent=CreateEvent(NULL,FALSE,FALSE,NULL);
  OutEvent=CreateEvent(NULL,FALSE,FALSE,NULL);
  IsInOpen=IsOutOpen=false;
  IsOutPaused=IsOutReset=false;
  InStor=NULL; OutStor=NULL;
  IN_DEVICE=WAVE_MAPPER;
  OUT_DEVICE=WAVE_MAPPER;
 // VolumeOut=INT_MAX;
}
//------------------------------------------------------------------------------
TSound::~TSound()
{
  closeIn(); closeOut();

  CloseHandle(OutEvent);
  CloseHandle(InEvent);
  DeleteCriticalSection(&SoundInCriticalSection);
  DeleteCriticalSection(&SoundOutCriticalSection);
}
//------------------------------------------------------------------------------
bool TSound::openIn(int samples_per_sec, int bit_per_sample, int channels, int samples_buf_size)
{ /* Открывает звуковую карту на ввод
     sample_per_sec - частота дискретизации (отсчетов в секунду)
     bit_per_sample - разрядность отсчета (8 бит, 16 бит и т.д.)
     channels       - моно-стерео
     samples_buf_size - размер одного буфера ввода в отсчетах (для моно и стерео одинаково)
     Возвращает -     удалось ли открыть
  */
  int temp;
  MMRESULT result;

  if(IsInOpen) closeIn();

  waveInFormat.wFormatTag = WAVE_FORMAT_PCM;
  waveInFormat.nChannels = channels;
  waveInFormat.nSamplesPerSec = samples_per_sec;
  waveInFormat.wBitsPerSample = bit_per_sample;
  temp = waveInFormat.nChannels * waveInFormat.wBitsPerSample;
  temp = temp/8;
  waveInFormat.nBlockAlign =(WORD) temp;
  waveInFormat.nAvgBytesPerSec = waveInFormat.nSamplesPerSec * waveInFormat.nBlockAlign;
  waveInFormat.cbSize = 0;

  InBufSize=samples_buf_size*channels*bit_per_sample/(8*sizeof(char));

  result = waveInOpen(&inHandle, IN_DEVICE, &waveInFormat,
                (DWORD)WaveInProc,(DWORD)this, CALLBACK_FUNCTION);
  if(result)
  {  MessageBox(NULL, "Cannot Open Audio Input Device","DDS Control",
      MB_ICONEXCLAMATION | MB_OK);
      IsInOpen=false;
     return IsInOpen;
  } else IsInOpen=true;

  // выделение памяти
  ZeroMemory(waveInHeader,sizeof(waveInHeader));
  InStor=(unsigned char*)VirtualAlloc(0,InBufSize*NINBUFS, MEM_COMMIT, PAGE_READWRITE);
  if(InStor==NULL){closeIn(); return IsInOpen;}
  ZeroMemory(InStor,InBufSize*NINBUFS);

  curentInBuf=0; posInBuf=0; InBufCnt=0; InOverflow=false;
  for(int i=0;i<NINBUFS;i++)
  {
   InDataReady[i]=i;
   waveInHeader[i].dwUser = i;
   waveInHeader[i].lpData = InStor + InBufSize*i;
   waveInHeader[i].dwBufferLength = InBufSize;
   waveInPrepareHeader(inHandle, &waveInHeader[i], sizeof(WAVEHDR));
   waveInAddBuffer(inHandle,&waveInHeader[i],sizeof(WAVEHDR));
  }
  waveInStart(inHandle);

  return IsInOpen;
}
//------------------------------------------------------------------------------
bool TSound::openOut(int samples_per_sec, int bit_per_sample, int channels, int samples_buf_size)
{ /* Открывает звуковую карту на вывод
     sample_per_sec - частота дискретизации (отсчетов в секунду)
     bit_per_sample - разрядность отсчета (8 бит, 16 бит и т.д.)
     channels       - моно-стерео
     samples_buf_size - размер одного буфера ввода в отсчетах (для моно и стерео одинаково)
     Возвращает -     удалось ли открыть
  */
  int temp;
  MMRESULT result;

  if(IsOutOpen) closeOut();

  waveOutFormat.wFormatTag = WAVE_FORMAT_PCM;
  waveOutFormat.nChannels = channels;
  waveOutFormat.nSamplesPerSec = samples_per_sec;
  waveOutFormat.wBitsPerSample = bit_per_sample;
  temp = waveOutFormat.nChannels * waveOutFormat.wBitsPerSample;
  temp = temp/8;
  waveOutFormat.nBlockAlign =(WORD) temp;
  waveOutFormat.nAvgBytesPerSec = waveOutFormat.nSamplesPerSec * waveOutFormat.nBlockAlign;
  waveOutFormat.cbSize = 0;

  OutBufSize=samples_buf_size*channels*bit_per_sample/(8*sizeof(char));

  result = waveOutOpen(&outHandle, OUT_DEVICE, &waveOutFormat,
                  (DWORD)WaveOutProc, (DWORD)this, CALLBACK_FUNCTION);

  if(result)
  {  MessageBox(NULL, "Cannot Open Audio Output Device","DDS Control",
      MB_ICONEXCLAMATION | MB_OK);
     IsOutOpen=false;
     return IsOutOpen;
  } else IsOutOpen=true;

  OutStor=(unsigned char*)VirtualAlloc(0,OutBufSize*NOUTBUFS, MEM_COMMIT, PAGE_READWRITE);
  if(OutStor==NULL){closeOut();return IsOutOpen;}
  ZeroMemory(OutStor,OutBufSize*NOUTBUFS);

  for(int i=0;i<NOUTBUFS;i++)
  {
   OutDataEmpty[i]=true;
   waveOutHeader[i].dwUser = i;
   waveOutHeader[i].lpData = OutStor + OutBufSize*i;
   waveOutHeader[i].dwBufferLength = OutBufSize;
   waveOutHeader[i].dwFlags=0;
   waveOutPrepareHeader(outHandle, &waveOutHeader[i], sizeof(WAVEHDR));
  }
  curentOutBuf=0; posOutBuf=0; OutUnderflow=false;
  IsOutPaused=IsOutReset=false;
  waveOutGetVolume(outHandle,&VolumeOut);
  return IsOutOpen;
}
//------------------------------------------------------------------------------
void TSound::closeIn(void)
{
  if(!IsInOpen)return;

  waveInStop(inHandle);
  waveInReset(inHandle);
  for(int i=0; i<NINBUFS; i++)
    waveInUnprepareHeader(inHandle, &waveInHeader[i], sizeof(WAVEHDR));
  // освободить память
  if(InStor!=NULL)
  {
   VirtualFree(InStor,InBufSize * NINBUFS, MEM_FREE);
   InStor==NULL;
  }
  waveInClose(inHandle); inHandle=NULL;
  IsInOpen=false;
}
//------------------------------------------------------------------------------
void TSound::closeOut(void)
{
  if(!IsOutOpen)return;
  
  waveOutReset(outHandle);
  waveOutClose(outHandle);
  for(int i=0; i<NOUTBUFS; i++)
    waveOutUnprepareHeader(outHandle, &waveOutHeader[i], sizeof(WAVEHDR));
  outHandle=NULL;
  // освободить память
  if(OutStor!=NULL)
  {
   VirtualFree(OutStor,OutBufSize * NOUTBUFS, MEM_FREE);
   OutStor=NULL;
  }
  IsOutOpen=false;
}
//------------------------------------------------------------------------------
int TSound::read(float * left_samples, float * right_samples, int n_samples)
{ /* Читает готовые отсчеты со звуковой карты
     left_samples - указатель, куда  складывать отсчеты левого канала
     right_samples - указатель, куда  складывать отсчеты правого канала
     n_samples - сколько отсчетов нужно
     Если моно, то каналы могут быть одинаковыми, либо один из них=NULL
     возвращает - кол-во байт(по одному каналу), оставшееся в буфере
     Функция блокируется, пока данные не готовы
  */
   float x[2];
   int Umax=(1<<(waveInFormat.wBitsPerSample-1)) + 2;
   int MAX_WAIT_TIME=1000*InBufSize/waveInFormat.nAvgBytesPerSec;

   if(!IsInOpen)return 0;
   if(left_samples==NULL && right_samples==NULL) return 0;
   for(int i=0; i<n_samples; i++)
   {
      if(posInBuf>=InBufSize)  // буфер кончился, ждем новый
      {
       waveInPrepareHeader(inHandle, &waveInHeader[curentInBuf],sizeof(WAVEHDR));
       waveInAddBuffer(inHandle,&waveInHeader[curentInBuf],sizeof(WAVEHDR));
       try{
         EnterCriticalSection(&SoundInCriticalSection);
         for(int j=1;j<NINBUFS;j++)InDataReady[j-1] = InDataReady[j];
         InBufCnt--; ResetEvent(InEvent);
          }
       __finally { LeaveCriticalSection(&SoundInCriticalSection);}
       while(posInBuf>=InBufSize)
       {if(InBufCnt>0) {curentInBuf=InDataReady[0]; posInBuf=0;}
        else WaitForSingleObject(InEvent,MAX_WAIT_TIME);
       }
      }
      
      // собственно чтение отсчетов
      for(int k=0; k<waveInFormat.nChannels; k++)
      {
        if(waveOutFormat.wBitsPerSample==16)
        { short int y;
          y=InStor[curentInBuf*InBufSize+posInBuf];
          y=y | ((short int)(InStor[curentInBuf*InBufSize+posInBuf+1])<<8);
          posInBuf=posInBuf+2;
          x[k]=((float)y)/Umax;
        }
        else
        { unsigned char y;
          y=InStor[curentInBuf*InBufSize+posInBuf];
          posInBuf++;
          x[k]=(float)(y-128)/Umax;
        }
      }

      if(waveInFormat.nChannels==1)
      { if(left_samples!=NULL)left_samples[i]=x[0];
        if(right_samples!=NULL)right_samples[i]=x[0];
      }
      else
      { left_samples[i]=x[0];
        right_samples[i]=x[1];
      }
   }
   return (InBufSize-posInBuf)/waveInFormat.nChannels;;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
int TSound::write(float * left_samples, float * right_samples, int n_samples)
{ /* Записывает готовые отсчеты в звуковую карту
     left_samples - отсчеты левого канала   | должны быть нормированы
     right_samples - отсчеты правого канала | к 1.0
     n_samples - сколько их всего
     Если моно, то каналы могут быть одинаковыми, либо один из них=NULL
     возвращает - кол-во байт, сколько осталось до заполнения буфера  (1 канал)
     функция блокируется до тех пор, пока данные не будут отданы драйверу
  */
  float x[2];
  int Umax=(1<<(waveOutFormat.wBitsPerSample-1)) + 2;
  int MAX_WAIT_TIME=1000*OutBufSize/waveOutFormat.nAvgBytesPerSec;

  if(!IsOutOpen)return 0;
  if(left_samples==NULL && right_samples==NULL) return 0;

  for(int i=0; i<n_samples; i++)
  {
    if(waveOutFormat.nChannels==2){x[0]=left_samples[i]; x[1]=right_samples[i];}
    else {
          if(left_samples!=NULL)x[0]=x[1]=left_samples[i];
          else x[0]=x[1]=right_samples[i];
         }

    for(int k=0; k<waveOutFormat.nChannels; k++)
    {
      if(waveOutFormat.wBitsPerSample==16)
      {  short int y=x[k]*Umax;
         /// записать в OutStor
         OutStor[curentOutBuf*OutBufSize+posOutBuf]=(unsigned char)(y&0x00FF);
         OutStor[curentOutBuf*OutBufSize+posOutBuf+1]=(unsigned char)((y&0xFF00)>>8);
         posOutBuf=posOutBuf+2;
      }
      else
      { unsigned char y=x[k]*Umax+128;
        /// записать в OutStor
        OutStor[curentOutBuf*OutBufSize+posOutBuf]=y;
        posOutBuf++;
      }
    }
    if(posOutBuf>=OutBufSize)
    {
      waveOutPrepareHeader(outHandle, &waveOutHeader[curentOutBuf],sizeof(WAVEHDR));
      waveOutWrite(outHandle,&waveOutHeader[curentOutBuf],sizeof(WAVEHDR));
      try{
        EnterCriticalSection(&SoundOutCriticalSection);
        OutDataEmpty[curentOutBuf]=false;
         }
      __finally
      { LeaveCriticalSection(&SoundOutCriticalSection);
      }

      while(posOutBuf>=OutBufSize)
      { try{
           EnterCriticalSection(&SoundOutCriticalSection);
           for(curentOutBuf=0; curentOutBuf<NOUTBUFS; curentOutBuf++)
             if(OutDataEmpty[curentOutBuf]){posOutBuf=0; break;}
           if(posOutBuf>=OutBufSize) ResetEvent(OutEvent);
           }
        __finally
       { LeaveCriticalSection(&SoundOutCriticalSection);
       }
        if(posOutBuf>=OutBufSize) WaitForSingleObject(OutEvent,MAX_WAIT_TIME);
      }
    }
  }
   IsOutReset=false;
   return (OutBufSize-posOutBuf)/waveOutFormat.nChannels;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void CALLBACK WaveOutProc(HWAVEOUT outHandle, UINT uMsg, TSound * snd,
                          DWORD dwParam1, DWORD dwParam2)
{
 DWORD bufno;
 WAVEHDR *wve;

 if(uMsg == WOM_DONE)
 { // find out which buffer has come back
   wve = (WAVEHDR*) dwParam1; // pointer to buffer
   bufno = wve->dwUser;       // dereference it
   try{
    EnterCriticalSection(&snd->SoundOutCriticalSection);
    snd->OutDataEmpty[bufno] = true;
    SetEvent(snd->OutEvent);
      }
    __finally
    { LeaveCriticalSection(&snd->SoundOutCriticalSection);
    }
 }
}
//------------------------------------------------------------------------------
void CALLBACK WaveInProc(HWAVEIN inHandle, UINT uMsg, TSound *snd,
                         DWORD dwParam1, DWORD dwParam2)
{
  WAVEHDR *wve;
  if(uMsg == MM_WIM_DATA)
  {
   // find out which buffer has come back
   wve = (WAVEHDR*) dwParam1; // pointer to buffer
   try{
    EnterCriticalSection(&snd->SoundInCriticalSection);
    if(snd->InBufCnt >= NINBUFS) {snd->InBufCnt=NINBUFS-1; snd->InOverflow=true;}
    else snd->InOverflow=false;
    snd->InDataReady[snd->InBufCnt] = (int) (wve->dwUser);
    if(snd->InBufCnt < NINBUFS) snd->InBufCnt++;
    SetEvent(snd->InEvent);
      }
    __finally
    { LeaveCriticalSection(&snd->SoundInCriticalSection);
    }
  }
}
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
bool TSound::openOutLoop(int samples_per_sec, int bit_per_sample, int channels, int samples_buf_size, bool isLoop, int n_loop_blocks)
{
        //TODO: Add your source code here
        /* Открывает звуковую карту на вывод
     sample_per_sec - частота дискретизации (отсчетов в секунду)
     bit_per_sample - разрядность отсчета (8 бит, 16 бит и т.д.)
     channels       - моно-стерео
     samples_buf_size - размер одного буфера ввода в отсчетах (для моно и стерео одинаково)
     isLoop - воспроизводить циклически
     n_loop-blocks -  количество блоков в цикле
     Возвращает -     удалось ли открыть
  */
  int temp;
  MMRESULT result;

  if(IsOutOpen) closeOut();

  waveOutFormat.wFormatTag = WAVE_FORMAT_PCM;
  waveOutFormat.nChannels = channels;
  waveOutFormat.nSamplesPerSec = samples_per_sec;
  waveOutFormat.wBitsPerSample = bit_per_sample;
  temp = waveOutFormat.nChannels * waveOutFormat.wBitsPerSample;
  temp = temp/8;
  waveOutFormat.nBlockAlign =(WORD) temp;
  waveOutFormat.nAvgBytesPerSec = waveOutFormat.nSamplesPerSec * waveOutFormat.nBlockAlign;
  waveOutFormat.cbSize = 0;

  OutBufSize=samples_buf_size*channels*bit_per_sample/(8*sizeof(char));

  result = waveOutOpen(&outHandle, OUT_DEVICE, &waveOutFormat,
                  (DWORD)WaveOutProc, (DWORD)this, CALLBACK_FUNCTION);

  if(result)
  {  MessageBox(NULL, "Cannot Open Audio Output Device","DDS Control",
      MB_ICONEXCLAMATION | MB_OK);
     IsOutOpen=false;
     return IsOutOpen;
  } else IsOutOpen=true;

  OutStor=(unsigned char*)VirtualAlloc(0,OutBufSize*NOUTBUFS, MEM_COMMIT, PAGE_READWRITE);
  if(OutStor==NULL){closeOut();return IsOutOpen;}
  ZeroMemory(OutStor,OutBufSize*NOUTBUFS);

  if(n_loop_blocks>NOUTBUFS) n_loop_blocks=NOUTBUFS;
  if(n_loop_blocks<1) n_loop_blocks=1;

  for(int i=0;i<NOUTBUFS;i++)
  {
   OutDataEmpty[i]=true;
   waveOutHeader[i].dwUser = i;
   waveOutHeader[i].lpData = OutStor + OutBufSize*i;
   waveOutHeader[i].dwBufferLength = OutBufSize;
   waveOutHeader[i].dwLoops=N_LOOP_MAX;
   waveOutHeader[i].dwFlags=0;
   if(i==0) waveOutHeader[i].dwFlags=WHDR_BEGINLOOP;
   if((i+1)==n_loop_blocks)waveOutHeader[i].dwFlags=waveOutHeader[i].dwFlags | WHDR_ENDLOOP;

   waveOutPrepareHeader(outHandle, &waveOutHeader[i], sizeof(WAVEHDR));
  }
  curentOutBuf=0; posOutBuf=0; OutUnderflow=false;

  return IsOutOpen;
}
//------------------------------------------------------------------------------


bool TSound::pauseOut(void)
{
 int r=0;
 if(IsOutPaused) return true;
 //waveOutGetVolume(outHandle,&VolumeOut);
 //waveOutSetVolume(outHandle,0);
 r=waveOutPause(outHandle);
 if(r==MMSYSERR_NOERROR){ IsOutPaused=true; return true;}
 else return false;
}

bool TSound::restartOut(void)
{
 int r=0;
 if(!IsOutPaused) return true;
 r=waveOutRestart(outHandle);
 //waveOutSetVolume(outHandle,VolumeOut);
 if(r==MMSYSERR_NOERROR) {IsOutPaused=false;return true;}
 else return false;
}

bool TSound::resetOut(void)
{
  int r=0;
  if(IsOutReset)return true;
 waveOutGetVolume(outHandle,&VolumeOut);
 //waveOutSetVolume(outHandle,0.8*VolumeOut);
 //waveOutSetVolume(outHandle,0.5*VolumeOut);
 //waveOutSetVolume(outHandle,0.3*VolumeOut);
 //waveOutSetVolume(outHandle,0.1*VolumeOut);
 waveOutSetVolume(outHandle,0);
 r=waveOutReset(outHandle);
 waveOutSetVolume(outHandle,VolumeOut);
 if(r==MMSYSERR_NOERROR){ IsOutReset=true; return true;}
 else return false;
}
