//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "WaveFile.h"

//---------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/////////////////// ������ �� ��������� ������� ////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
bool TWaveFile::open(AnsiString filename, AnsiString mode)
{ /* ��������� WAV-���� �� ������ ��� ������
     filename - ��� ����� (����)
     mode     - �����: "W" - ������, "R" - ������
     ���������� -  true - �������
                   false - ��������
  */

  DWORD dwFmtSize;

  if(opened){mmioClose(hmmio, 0); opened=false;}// ��� ��� ������ ������ ����

  if(mode.UpperCase() == "W") // �������� �� ������
  { IsWrite=true;
    hmmio=mmioOpen(filename.c_str(), NULL, MMIO_CREATE|MMIO_READWRITE);
    if(hmmio==NULL){opened=false; return opened;}
    mmckinfoParent.fccType=mmioFOURCC('W','A','V','E');
    mmioCreateChunk(hmmio, &mmckinfoParent, MMIO_CREATERIFF);
    mmckinfoSubchunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
    mmckinfoSubchunk.cksize = sizeof(WAVEFORMATEX);  // we know the size of this ck.
    if( mmioCreateChunk(hmmio, &mmckinfoSubchunk, 0) != 0)
    { mmioClose(hmmio,0); opened=false; return opened;}
    if( mmioWrite(hmmio, (HPSTR)&Format, sizeof(WAVEFORMATEX))!= sizeof(WAVEFORMATEX))
    { mmioClose(hmmio,0); opened=false; return opened;}
    mmioAscend(hmmio, &mmckinfoSubchunk, 0);
    mmckinfoSubchunk.cksize=0L;
    mmckinfoSubchunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
    if (mmioCreateChunk(hmmio, &mmckinfoSubchunk, 0) != 0)
    { mmioClose(hmmio,0);opened=false; return opened;}
    opened=true;
  }

  if(mode.UpperCase() == "R") // �������� �� ������
  { IsWrite=false;
    if(!(hmmio=mmioOpen(filename.c_str(), NULL, MMIO_READ|MMIO_ALLOCBUF)))
    { // ������ ��� ��������
      AvailableRead=opened=false; return opened;
    }
    mmckinfoParent.fccType=mmioFOURCC('W','A','V','E');
    if(mmioDescend(hmmio, (LPMMCKINFO) &mmckinfoParent, NULL, MMIO_FINDRIFF))
    { // ��� �� ���������
      mmioClose(hmmio, 0); AvailableRead=opened=false; return opened;
    }
    mmckinfoSubchunk.ckid=mmioFOURCC('f','m','t',' ');
    if(mmioDescend(hmmio, &mmckinfoSubchunk, &mmckinfoParent, MMIO_FINDCHUNK))
    { // ��� �������� �������
      mmioClose(hmmio, 0); AvailableRead=opened=false; return opened;
    }
    dwFmtSize=mmckinfoSubchunk.cksize;
    if(mmioRead(hmmio, (HPSTR)&Format, dwFmtSize) != dwFmtSize)
    { // ������ �� ��������
      mmioClose(hmmio, 0); AvailableRead=opened=false; return opened;
    }
    mmioAscend(hmmio, &mmckinfoSubchunk, 0);
    mmckinfoSubchunk.cksize=0L;
    mmckinfoSubchunk.ckid=mmioFOURCC('d','a','t','a');
    if(mmioDescend(hmmio, &mmckinfoSubchunk, &mmckinfoParent, MMIO_FINDCHUNK))
    { // ��� ����� ������
      mmioClose(hmmio, 0); AvailableRead=opened=false; return opened;
    }
    dwDataSize=mmckinfoSubchunk.cksize;
    if(dwDataSize == 0L){mmioClose(hmmio, 0); opened=false; return opened;}
    opened=true;  AvailableRead=true;
  }
  return opened;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
TWaveFile::~TWaveFile()
{
  close();
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
TWaveFile::TWaveFile()
{
  opened=false;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void TWaveFile::close(void)
{
  if(opened) {
   if(IsWrite)
   {  //mmioFlush(hmmio,0);
      // Ascend out of the 'data' chunk, back into the 'RIFF' chunk.
      mmioAscend(hmmio, &mmckinfoSubchunk, 0);
       // Ascend out of the 'RIFF' chunk, writing the RIFF file length.
      mmioAscend(hmmio, &mmckinfoParent, 0);
   }
   mmioClose(hmmio, 0); AvailableRead=opened=false;
             }
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
bool TWaveFile::write(char * data, long len)
{ /* ���������� ������ � WAV ����
     data - ����� �������
     len - ����� ������� data
  */
  if(!opened) return false;
  if(mmioWrite(hmmio, data, len)!=len) return false;
  return true;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
long TWaveFile::read(char * data, long len)
{ /* ������ ������ �� WAV-�����
     data - ������ ��� ��������� ������
     len  - ����� �������
     ���������� - ���������� ��������� ������
  */
   long cnt=0;
   if(!opened) return -1;
   cnt=mmioRead(hmmio, data, len);
   if(cnt>0) AvailableRead=true; else AvailableRead=false;
   return cnt;
}
//------------------------------------------------------------------------------
int TWaveFile::read(float * left_samples, float * right_samples, int n_samples)
{ /*  ������ �� ����� �������
      left_samples  - ���� ������ ��������� ������� ������ ������
      right_samples - ���� ������ ��������� ������� ������� ������
      n_samples     - ������� �������� ��������� �������
      ����������    - ���������� ���������� ��������� ��������
      ���� ����, �� ���� left_samples, ����  right_samples ����� =NULL
  */
  float x[2];
  int cnt=0, r=0;
  int Umax=(1<<(Format.wBitsPerSample-1)) + 2;
  
  if(!opened) return -1;
  if(left_samples==NULL && right_samples==NULL) return 0;

  int n_bytes=Format.nBlockAlign * n_samples;
  unsigned char *data=new unsigned char[n_bytes];
  r=mmioRead(hmmio, data, n_bytes);
  n_samples=r/Format.nBlockAlign;

  for(int i=0; i<n_samples; i++)
  {
    for(int k=0; k<Format.nChannels; k++)
    {
     if(Format.wBitsPerSample==16)
     { short int y;
       y=data[cnt] | ((short int)(data[cnt+1])<<8);
       cnt=cnt+2;
       x[k]=(float)y/Umax;
     }
     else
     { unsigned char y;
       y=data[cnt]; cnt++;
       x[k]=(float)(y-128)/Umax;
     }
    }

   if(Format.nChannels==1)
   { if(left_samples!=NULL)left_samples[i]=x[0];
     if(right_samples!=NULL)right_samples[i]=x[0];
   }
   else
   { left_samples[i]=x[0];
     right_samples[i]=x[1];
   }
  }

  delete[] data;
  return n_samples;
}
//------------------------------------------------------------------------------
int TWaveFile::write(float * left_samples, float * right_samples, int n_samples)
{ /*  ���������� ������� � ����
      left_samples  - ������� ������ ������
      right_samples - ������� ������� ������
      n_samples     - ������� �������� ��������� ��������
      ����������    - ���������� ���������� ���������� ��������
      ���� ����, �� ���� left_samples, ����  right_samples ����� =NULL
  */
  float x[2];
  int cnt=0, w=0;
  int Umax=(1<<(Format.wBitsPerSample-1)) + 2;

  if(!opened) return -1;
  if(left_samples==NULL && right_samples==NULL) return 0;

  int n_bytes=Format.nBlockAlign * n_samples;
  unsigned char *data=new unsigned char[n_bytes];

  for(int i=0; i<n_samples; i++)
  {
    if(Format.nChannels==2){x[0]=left_samples[i]; x[1]=right_samples[i];}
    else {
          if(left_samples!=NULL)x[0]=x[1]=left_samples[i];
          else x[0]=x[1]=right_samples[i];
         }

    for(int k=0; k<Format.nChannels; k++)
    {
      if(Format.wBitsPerSample==16)
      {  short int y=x[k]*Umax;
         data[cnt]=(unsigned char)(y&0x00FF);
         data[cnt+1]=(unsigned char)((y&0xFF00)>>8);
         cnt=cnt+2;
      }
      else
      { unsigned char y=x[k]*Umax+128;
        data[cnt]=y;
        cnt++;
      }
    }
  }
  w=mmioWrite(hmmio, data, cnt);
  delete[] data;
  return w/Format.nBlockAlign;
}
//------------------------------------------------------------------------------
void TWaveFile::setFormat(int samples_per_sec, int bit_per_sample, int n_channels)
{/* ������������� ��������� ��� ������� ��������� �����

 */
 int temp;
 Format.nSamplesPerSec=samples_per_sec;
 Format.wBitsPerSample=bit_per_sample;
 Format.nChannels=n_channels;

 Format.wFormatTag = WAVE_FORMAT_PCM;
  temp = Format.nChannels * Format.wBitsPerSample;
  temp = temp/8;
 Format.nBlockAlign =(WORD) temp;
 Format.nAvgBytesPerSec = Format.nSamplesPerSec * Format.nBlockAlign;
 Format.cbSize = 0;
 return;
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#pragma package(smart_init)
