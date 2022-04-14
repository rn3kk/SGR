//---------------------------------------------------------------------------

#ifndef WaveFileH
#define WaveFileH
#include <mmsystem.h>
#include <mem.h>
#include <values.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/////////////////// –¿¡Œ“¿ —Œ «¬” Œ¬€Ã» ‘¿…À¿Ã» ////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class TWaveFile
{ protected:
  private:
   bool opened;
   bool IsWrite;
   HMMIO hmmio;
   DWORD dwDataSize;
   MMCKINFO mmckinfoParent, mmckinfoSubchunk;

  public:
        bool AvailableRead;
        WAVEFORMATEX Format;
        bool open(AnsiString filename, AnsiString mode);
        ~TWaveFile();
        TWaveFile();
        void close(void);
        bool write(char * data, long len);
        long read(char * data, long len);
        int read(float * left_samples, float * right_samples, int n_samples);
        int write(float * left_samples, float * right_samples, int n_samples);
        void setFormat(int samples_per_sec, int bit_per_sample, int n_channels);
};
//---------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
#endif
