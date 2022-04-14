//---------------------------------------------------------------------------

#ifndef sgr1H
#define sgr1H
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "WaveFile.h"
#include "Sound.h"
#include <Menus.hpp>
#include <inifiles.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mem.h>
#include <Graphics.hpp>
#include <Dialogs.hpp>
#include <Buttons.hpp>
#include <Filectrl.hpp>
#include <jpeg.hpp>

//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// IDE-managed Components
        TMainMenu *MainMenu1;
        TMenuItem *File1;
        TMenuItem *Device1;
        TMenuItem *device01;
        TMenuItem *device11;
        TMenuItem *device21;
        TMenuItem *device31;
        TMenuItem *device41;
        TMenuItem *device51;
        TMenuItem *device61;
        TMenuItem *device71;
        TMenuItem *device81;
        TImage *Image1;
        TStatusBar *StatusBar1;
        TMenuItem *N1;
        TMenuItem *default1;
        TTimer *Timer1;
        TTimer *Timer2;
        TMenuItem *Capture1;
        TMenuItem *N5min1;
        TMenuItem *N10min1;
        TMenuItem *N15min1;
        TMenuItem *N20min1;
        TMenuItem *N30min1;
        TMenuItem *disabled1;
        TMenuItem *userperiod1;
        TImage *Image2;
        TMenuItem *N1hour1;
        TMenuItem *Calibration1;
        TMenuItem *autoDCFHGA1;
        TMenuItem *fixedvalue1;
        TMenuItem *N2;
        TMenuItem *enterdF1;
        TOpenDialog *OpenDialog1;
        TMenuItem *Openwav1;
        TMenuItem *N3;
        TMenuItem *Exit1;
        TTrackBar *TrackBar1;
        TLabel *Label1;
        TSpeedButton *SpeedButton1;
        TSpeedButton *SpeedButton2;
        TLabel *Label2;
        TMenuItem *N1min1;
        TMenuItem *N3min1;
        TMenuItem *NB1;
        TMenuItem *enable1;
        TMenuItem *N4;
        TMenuItem *Level1;
        TMenuItem *N6dB1;
        TMenuItem *N10dB1;
        TMenuItem *N15dB1;
        TMenuItem *N20dB1;
        TMenuItem *N30dB1;
        TMenuItem *user1;
        TMenuItem *N3dB1;
        TEdit *Edit1;
        TLabel *Label3;
        TSpeedButton *SpeedButton3;
        TMenuItem *N5;
        TMenuItem *BMP1;
        TMenuItem *JPG1;
        TMenuItem *Freq1;
        TMenuItem *Colors1;
        TMenuItem *Help1;
        TMenuItem *Fdial1;
        TMenuItem *N1360001;
        TMenuItem *N1350002;
        TMenuItem *N1355002;
        TMenuItem *N1370001;
        TMenuItem *N1250001;
        TMenuItem *user2;
        TMenuItem *N6;
        TMenuItem *EnterFdial1;
        TMenuItem *N7;
        TMenuItem *EU1;
        TMenuItem *TA1;
        TMenuItem *user3;
        TMenuItem *EnterFc1;
        TMenuItem *N8;
        TMenuItem *N9;
        TMenuItem *DCF391;
        TMenuItem *HGA221;
        TColorDialog *ColorDialog1;
        TMenuItem *Spectrum1;
        TMenuItem *SlowSpectrum1;
        TMenuItem *Backgfound1;
        TMenuItem *N10;
        TMenuItem *default2;
        TMenuItem *N11;
        TMenuItem *Upload1;
        TMenuItem *Waterfall1;
        TMenuItem *Simple1;
        TMenuItem *withStacking1;
        TImage *Image3;
        TEdit *Edit2;
        TEdit *Edit3;
        TLabel *Label4;
        TSpeedButton *SpeedButton4;
        TMenuItem *N12;
        TMenuItem *CALLandQTH1;
        TMenuItem *HowsetupCALLandQTH1;
        TMenuItem *CaptureUploadhelp1;
        TMenuItem *Calibrationhelp1;
        TMenuItem *NoiseBlankerhelp1;
        TMenuItem *Frequencyhelp1;
        TMenuItem *AboutWaterfallStacking1;
        TMenuItem *N13;
        TMenuItem *AboutSGRA1;
        void __fastcall device01Click(TObject *Sender);
        void __fastcall device11Click(TObject *Sender);
        void __fastcall device21Click(TObject *Sender);
        void __fastcall device31Click(TObject *Sender);
        void __fastcall device41Click(TObject *Sender);
        void __fastcall device51Click(TObject *Sender);
        void __fastcall device61Click(TObject *Sender);
        void __fastcall device71Click(TObject *Sender);
        void __fastcall device81Click(TObject *Sender);
        void __fastcall default1Click(TObject *Sender);
        void __fastcall Timer1Timer(TObject *Sender);
        void __fastcall Timer2Timer(TObject *Sender);
        void __fastcall disabled1Click(TObject *Sender);
        void __fastcall N5min1Click(TObject *Sender);
        void __fastcall N10min1Click(TObject *Sender);
        void __fastcall N15min1Click(TObject *Sender);
        void __fastcall N20min1Click(TObject *Sender);
        void __fastcall N30min1Click(TObject *Sender);
        void __fastcall userperiod1Click(TObject *Sender);
        void __fastcall Image2MouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
        void __fastcall N1hour1Click(TObject *Sender);
        void __fastcall Openwav1Click(TObject *Sender);
        void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
        void __fastcall autoDCFHGA1Click(TObject *Sender);
        void __fastcall fixedvalue1Click(TObject *Sender);
        void __fastcall TrackBar1Change(TObject *Sender);
        void __fastcall SpeedButton2Click(TObject *Sender);
        void __fastcall enable1Click(TObject *Sender);
        void __fastcall N6dB1Click(TObject *Sender);
        void __fastcall N10dB1Click(TObject *Sender);
        void __fastcall N15dB1Click(TObject *Sender);
        void __fastcall N20dB1Click(TObject *Sender);
        void __fastcall N30dB1Click(TObject *Sender);
        void __fastcall user1Click(TObject *Sender);
        void __fastcall N3dB1Click(TObject *Sender);
        void __fastcall N1min1Click(TObject *Sender);
        void __fastcall N3min1Click(TObject *Sender);
        void __fastcall enterdF1Click(TObject *Sender);
        void __fastcall SpeedButton3Click(TObject *Sender);
        void __fastcall Edit1Change(TObject *Sender);
        void __fastcall Edit1KeyPress(TObject *Sender, char &Key);
        void __fastcall BMP1Click(TObject *Sender);
        void __fastcall JPG1Click(TObject *Sender);
        void __fastcall EnterFdial1Click(TObject *Sender);
        void __fastcall EnterFc1Click(TObject *Sender);
        void __fastcall TA1Click(TObject *Sender);
        void __fastcall EU1Click(TObject *Sender);
        void __fastcall N1350002Click(TObject *Sender);
        void __fastcall N1355002Click(TObject *Sender);
        void __fastcall N1360001Click(TObject *Sender);
        void __fastcall N1370001Click(TObject *Sender);
        void __fastcall N1250001Click(TObject *Sender);
        void __fastcall user3Click(TObject *Sender);
        void __fastcall HGA221Click(TObject *Sender);
        void __fastcall DCF391Click(TObject *Sender);
        void __fastcall Spectrum1Click(TObject *Sender);
        void __fastcall SlowSpectrum1Click(TObject *Sender);
        void __fastcall Backgfound1Click(TObject *Sender);
        void __fastcall default2Click(TObject *Sender);
        void __fastcall SpeedButton1Click(TObject *Sender);
        void __fastcall Upload1Click(TObject *Sender);
        void __fastcall Simple1Click(TObject *Sender);
        void __fastcall withStacking1Click(TObject *Sender);
        void __fastcall Exit1Click(TObject *Sender);
        void __fastcall Image3MouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
        void __fastcall Image3MouseMove(TObject *Sender, TShiftState Shift,
          int X, int Y);
        void __fastcall Image3MouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
        void __fastcall SpeedButton4Click(TObject *Sender);
        void __fastcall Edit2Enter(TObject *Sender);
        void __fastcall CALLandQTH1Click(TObject *Sender);
        void __fastcall HowsetupCALLandQTH1Click(TObject *Sender);
        void __fastcall CaptureUploadhelp1Click(TObject *Sender);
        void __fastcall Calibrationhelp1Click(TObject *Sender);
        void __fastcall NoiseBlankerhelp1Click(TObject *Sender);
        void __fastcall Frequencyhelp1Click(TObject *Sender);
        void __fastcall AboutWaterfallStacking1Click(TObject *Sender);
        void __fastcall AboutSGRA1Click(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TForm1(TComponent* Owner);
        int paintSpectrum(int width);
        int paintFreqScale(int x_spectrum_start);
        rotateText(int angle);
        paintSpectrogram(int x_start);
        void changeF(double Fdial, double Fc);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
