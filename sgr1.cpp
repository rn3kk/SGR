//---------------------------------------------------------------------------

#include <vcl.h>
#include <string>
#pragma hdrstop




#include "filters.h"
#include "TIIRLPF.h"
#include "TIIRBPF2.h"
#include "common.h"
#include "sgr1.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

#define SND_FD 12000  //12000
#define SND_LEN 12000 //12000
#define SND_FD2 48000
#define SND_LEN2 48000
//#define CLIP_POROG 6  //во сколько раз импульс помехи превышает среднее значение
#define DF 0.051//0.017 //полоса пропускания [Гц] фильтра по уровню 0.7
#define NF 412// //  кол-во фильтров, не менее 412
#define K12 24      // первая децимация  до 500 Гц
#define KDECIM 5000 // вторая децимация  до 0.1 Гц
#define LEN_WF 720 // буфер водопада на два часа при обновлении раз в 10 сек
#define LEN_HOUR 360//360 // длина буфера на один час с накоплением при обновлении раз в 10 сек
#define BW_F 20.0 // фильтр частотомера
#define DF_LEN 1000 //усреднение частоты

TForm1 *Form1;
TSound *snd;
TWaveFile *wf, *rec;
TIIRBPF2 * BPF[NF+2], * BPF2[NF+2], * BPF3[NF+2];
TIIRLPF  * LPF[NF+2];
TIIRBPF2 * BPF_F, * BPF_F1, * BPF_F11, * BPF_F2; // узкий фильтр для частотомера
TIIRBPF2 * filter1, * filter11, * filter111;
bool AUDIO_RX_WORKED;
HANDLE hAudioRx;
bool processWav;
float x[SND_LEN+2],x1[SND_LEN+2];
float x2[SND_LEN2+2];
// для частотомера//
float xf[SND_LEN+2], xf2[SND_LEN+2];
unsigned long CNT_Z=0; //счетчик переходов через ноль
unsigned long CNT_Z_ALL=0;//cчетчик обработанных отсчетов
double deltaF=0, d_deltaF=0, df0[DF_LEN+2];
double Uf=0, Uf2=0;
int cnt_F=0;
float SNRF;
////////////////////
double U[NF+2];
double u0[NF+2];
double Uslow[NF+2];
double Usr=0, Usred=0;
float WF[NF][LEN_WF+2];
float WFsred[LEN_WF+2];
float WF_HOUR[NF][LEN_HOUR+2]; //часовой накопитель
float WFHsred[LEN_HOUR+2];

double F0;  // центральная частота приема, например 1777 Гц
double Fget; // частота гетеродина для переноса в область 177 Гц
int cntClip;
unsigned int cntDecim;
double Fdial;   // частота настройки приемника, например 136000 Гц
double Fspan; // ширина полосы приема 1...20 Гц
double Flow,Fhigh; // границы окна приема (аудиочастоты), например 1775-1781 Гц
double ph, dph;
double ph2, dph2;
AnsiString mycall;
AnsiString myqth;
AnsiString CaptureFilename;
AnsiString CapturePath;
int CAPTURE_PERIOD;
double dbOffset;
double CONTRAST;
int SPECTRUM_W;

double CalibratedDF;
int CalibMenu;
bool REC;
double CLIP_POROG=6, NB_THRESHOLD=15;
bool NB_ENABLE;
int CAPTURE_FORMAT=0;
int OK_Button_mode=0;
double F_TA=137777.0, F_EU=136172.0;
double F_HGA22=135430.0, F_DCF39=138830.4;
double F0_user=1500;
bool USE_LSB=false;
bool SHOW_HOUR=false;
bool RECORDING=false;
long bytes_recorded=0;
AnsiString recfile;
TColor SpectrumColor, SlowSpectrumColor, BackColor;

AnsiString UploadExecStr;

int X1,X2,Y1,Y2,X_SCALE;



/////////////////////////////////////////////////////////////////////////////////
DWORD WINAPI AudioRxThread( LPVOID )
{
  ph=M_PI/256.0;
  ph2=M_PI/256.0;
  processWav=false;
  memset(x,0,sizeof(x));
  memset(x1,0,sizeof(x1));
  memset(x2,0,sizeof(x2));
  memset(xf,0,sizeof(xf));
  memset(xf2,0,sizeof(xf2));
  memset(df0,0,sizeof(df0));
  memset(U,0,sizeof(U));
  memset(Uslow,0,sizeof(Uslow));
  memset(WFsred,0,sizeof(WFsred));
  memset(WFHsred,0,sizeof(WFHsred));

  Usr=Usred=Uf=Uf2=0;

  while(1)
  {
    AUDIO_RX_WORKED=true;
    try
    {
      if(F0<5450)
      {
       snd->read(x,x,SND_LEN);
      }
      else ////////// 12500Hz
      {
        snd->read(x2,x2,SND_LEN2);
        // перенести с высокой F0 вниз
        convertFrom48000_toDown(x2, x, filter1, filter11, filter111, dph2);
      }////////////end 12500

      //найти мгновенный средний уровень
      std::string s = getAvgMediumLevel(x, Usr);
      Form1->StatusBar1->Panels->Items[2]->Text=s.c_str();

      // подавление импульсных помех
      cntClip=0;
      if(NB_ENABLE)
        cntClip = nb(x, Usr*CLIP_POROG);

      AnsiString snb; snb.sprintf("NB %d%%",100*cntClip/SND_LEN);
      if(cntClip>=0.01*SND_LEN)
        Form1->StatusBar1->Panels->Items[4]->Text=snb;
      else
        Form1->StatusBar1->Panels->Items[4]->Text="";

     // средний уровень после подавления импульсов
     float uu=0;
     for(int i=0; i<SND_LEN; i++)uu=uu+(x[i]*x[i]);
     uu=uu/SND_LEN;
     if(Usred==0) Usred=uu;
     Usred=0.95*Usred+0.05*uu;

 ///////////// частотомер //////////////////////////////////////////////////////

     freqCounter(x, xf, xf2, df0, deltaF, d_deltaF, BPF_F, BPF_F1, BPF_F2);

     AnsiString sf;
     if(d_deltaF>0)
     {
       if(Form1->autoDCFHGA1->Checked)
         sf.sprintf("dF=%.3f+-%.3f Hz",deltaF,d_deltaF);
       else
         sf.sprintf("dF=%.3f Hz (%.3f Hz)",CalibratedDF, deltaF);
     }
     else
       sf.sprintf("dF counting...");
     Form1->StatusBar1->Panels->Items[5]->Text=sf;
     if(Form1->autoDCFHGA1->Checked && d_deltaF>0 && d_deltaF<0.01)
     {
       CalibratedDF=deltaF;
       AnsiString sfv; sfv.sprintf("constant dF=%.3f Hz",CalibratedDF);
       Form1->fixedvalue1->Caption=sfv;
       if(!processWav)
         Form1->fixedvalue1Click(NULL);
       else
         Form1->fixedvalue1->Checked=true;
     }
     else if(!processWav)
       Form1->StatusBar1->Panels->Items[5]->Text="NO CARRIER";


 //////// конец частототмера /////////////////////////////////////

    //предварительно отфильтровать
    if(F0>737 && F0<817) for(int i=0; i<SND_LEN; i++) x1[i]=iir777_12k(x[i]);
    else
    { if(F0>1737 && F0<1817) for(int i=0; i<SND_LEN; i++) x1[i]=iir1777_12k(x[i]);
      else
      { if(F0>2737 && F0<2817) for(int i=0; i<SND_LEN; i++) x1[i]=iir2777_12k(x[i]);
        else
        { if(F0>2237 && F0<2317)for(int i=0; i<SND_LEN; i++) x1[i]=iir2277_12k(x[i]);
          else
          { if(F0>788 && F0<868) for(int i=0; i<SND_LEN; i++) x1[i]=iir828_12k(x[i]);
            else
            {if(F0>632 && F0<712) for(int i=0; i<SND_LEN; i++) x1[i]=iir672_12k(x[i]);
             else
             {  if(F0>1132 && F0<1212) for(int i=0; i<SND_LEN; i++) x1[i]=iir1172_12k(x[i]);
                else
                { if(F0<222) for(int i=0; i<SND_LEN; i++) x1[i]=iir172_12k(x[i]);
                  else
                  { if(F0<5450) for(int i=0; i<SND_LEN; i++) x1[i]=filter111->filter(filter11->filter(filter1->filter(x[i])));//filter1->filter(x,SND_LEN,x1);
                    else for(int i=0; i<SND_LEN; i++) x1[i]=iir1777_12k(x[i]);
                  }
                }
              }
            }
          }
        }
      }
    }
    // с учетом калиброванного частотного сдвига корректируем частоту гетеродина
     if(F0<5450) {Fget=F0+CalibratedDF-177.0;dph=TWOPI*(Fget)/SND_FD; dph2=0; }
     else {Fget=1777.0+CalibratedDF-177.0;dph=TWOPI*(Fget)/SND_FD; dph2=TWOPI*(F0-1777.0)/SND_FD2;}
    // перенести сигнал по частоте вниз
     for(int i=0; i<SND_LEN; i++){x1[i]=x1[i]*cos(ph); ph=ph+dph; if(ph>TWOPI)ph=ph-TWOPI;}
   // фильтрация
     for(int i=0; i<SND_LEN; i++) x1[i]=iirlpf200_12k(x1[i]);

     //  фильтровать сигнал
     for(int iii=0; iii<SND_LEN; iii=iii+K12)  // первая децимация до 500 Гц
     {
      // обработать отсчеты с выходов фильтров - продетектировать и сгладить
      for(int n=0; n<NF; n++)
      {double yy=BPF[n]->filter(x1[iii]);
       yy=BPF2[n]->filter(yy); yy=BPF3[n]->filter(yy);
       U[n]=LPF[n]->filter(yy*yy);
      }

      if(cntDecim%KDECIM==0)  // вторая децимация до 1 раз в 10 сек
      {memmove(&WFsred[1],&WFsred[0],(LEN_WF-1)*sizeof(float));
       memmove(&WFHsred[1],&WFHsred[0],(LEN_HOUR-1)*sizeof(float));
       WFsred[0]=0; WFHsred[0]=0;
       float wfs=0;
       float wfsh=0;
       for(int n=0; n<NF; n++)
       { Uslow[n]=0.995*Uslow[n]+0.005*U[n];
        // здесь готовим данные для картинки
        memmove(&WF[n][1],&WF[n][0],(LEN_WF-1)*sizeof(float));
        WF[n][0]=U[n];
        wfs+=U[n];

        //часовой накопитель
        float tmp=WF_HOUR[n][LEN_HOUR-1];
        memmove(&WF_HOUR[n][1],&WF_HOUR[n][0],(LEN_HOUR-1)*sizeof(float));
        WF_HOUR[n][0]=0.5*tmp+0.5*U[n];  // интегратор с утечкой
        wfsh+=WF_HOUR[n][0];
       }
       wfs=wfs/NF;  wfsh=wfsh/NF;
       //убрать большие спектральные максимумы
       int cnt_wfs=1;
       int cnt_wfsh=1;
       for(int n=0; n<NF; n++)
       { if((U[n]-wfs)/wfs<3.0) {WFsred[0]+=U[n];cnt_wfs++;}
         if((WF_HOUR[n][0]-wfsh)/wfsh<3.0) {WFHsred[0]+=WF_HOUR[n][0];cnt_wfsh++;}
       }
       WFsred[0]=WFsred[0]/cnt_wfs;  WFHsred[0]=WFHsred[0]/cnt_wfsh;
      }
      cntDecim++;
     }

   Sleep(2);
   }catch(...){}
  }
 }

//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
        : TForm(Owner)
{
  Form1->DoubleBuffered=true;

 TIniFile *ini;
 ini = new TIniFile(ChangeFileExt( Application->ExeName, ".INI" ));
  int sndIn=ini->ReadInteger("SOUND","DEVICE",-1);
  ini->WriteInteger("SOUND","DEVICE",sndIn);
  mycall=ini->ReadString("GRABBER","MYCALL", "");
  ini->WriteString("GRABBER","MYCALL",mycall);
  myqth=ini->ReadString("GRABBER","MYQTH", "");
  ini->WriteString("GRABBER","MYQTH",myqth);
  F0=ini->ReadFloat("GRABBER" ,"F_CENTER",1777.0);
  if(F0<150.0) F0=150.0; if(F0>23000.0) F0=23000.0;
  ini->WriteFloat("GRABBER" ,"F_CENTER",F0);
  Fspan=ini->ReadFloat("GRABBER" ,"FREQ_SPAN",7.0);
  if(Fspan<0) Fspan=1.0; if(Fspan>20) Fspan=20;
  ini->WriteFloat("GRABBER" ,"FREQ_SPAN",Fspan);
  Fdial=ini->ReadFloat("GRABBER" ,"F_DIAL",136000.0);
  ini->WriteFloat("GRABBER" ,"F_DIAL",Fdial);
  F_TA=ini->ReadFloat("GRABBER" ,"F_TA",137777.0);
  ini->WriteFloat("GRABBER" ,"F_TA",F_TA);
  F_EU=ini->ReadFloat("GRABBER" ,"F_EU",136172.0);
  ini->WriteFloat("GRABBER" ,"F_EU",F_EU);
  F_HGA22=ini->ReadFloat("GRABBER" ,"HGA22",135430.0);
  ini->WriteFloat("GRABBER" ,"HGA22",F_HGA22);
  F_DCF39=ini->ReadFloat("GRABBER" ,"DCF39",138830.4);
  ini->WriteFloat("GRABBER" ,"DCF39",F_DCF39);
  CalibratedDF=ini->ReadFloat("GRABBER" ,"CALIBRATED_DF",0.0);
  ini->WriteFloat("GRABBER" ,"CALIBRATED_DF",CalibratedDF);
  CalibMenu=ini->ReadInteger("GRABBER" ,"CALIBRATION",0);
  ini->WriteInteger("GRABBER" ,"CALIBRATION",CalibMenu);
  CAPTURE_PERIOD=ini->ReadInteger("GRABBER","CAPTURE_PERIOD",0);
  if(CAPTURE_PERIOD<0)CAPTURE_PERIOD=0;
  ini->WriteInteger("GRABBER","CAPTURE_PERIOD",CAPTURE_PERIOD);
  CAPTURE_FORMAT=ini->ReadInteger("GRABBER","CAPTURE_FORMAT",0);
  if(CAPTURE_FORMAT<0)CAPTURE_FORMAT=0;
  ini->WriteInteger("GRABBER","CAPTURE_FORMAT",CAPTURE_FORMAT);
  dbOffset=ini->ReadFloat("GRABBER" ,"DB_OFFSET",-10);
  ini->WriteFloat("GRABBER" ,"DB_OFFSET",dbOffset);
  CONTRAST=ini->ReadFloat("GRABBER" ,"CONTRAST",1.0);
  ini->WriteFloat("GRABBER" ,"CONTRAST",CONTRAST);
  SPECTRUM_W=ini->ReadInteger("GRABBER" ,"SPECTRUM_WIDTH",80);
  ini->WriteInteger("GRABBER" ,"SPECTRUM_WIDTH",SPECTRUM_W);
  CaptureFilename=ini->ReadString("GRABBER" ,"CAPTURE_FILENAME","capt.bmp");
  ini->WriteString("GRABBER" ,"CAPTURE_FILENAME",CaptureFilename);
  CapturePath=ini->ReadString("GRABBER","CAPTURE_PATH", "Capture\\");
  CapturePath=IncludeTrailingBackslash(CapturePath);
  if (!DirectoryExists(CapturePath))
  {
    if (!CreateDir(CapturePath))
      throw Exception("Cannot create "+CapturePath+" directory.");
  }
  ini->WriteString("GRABBER","CAPTURE_PATH", CapturePath);
  NB_ENABLE=ini->ReadBool("GRABBER","NB_ENABLE", true);
  ini->WriteBool("GRABBER","NB_ENABLE",NB_ENABLE);
  NB_THRESHOLD=ini->ReadFloat("GRABBER","NB_THRESHOLD", 15);
  ini->WriteFloat("GRABBER","NB_THRESHOLD",NB_THRESHOLD);
  CLIP_POROG= pow(10,NB_THRESHOLD/20.0);

  SpectrumColor= ini->ReadInteger("COLORS","SPECTRUM",clYellow);
  ini->WriteInteger("COLORS","SPECTRUM",SpectrumColor);
  SlowSpectrumColor= ini->ReadInteger("COLORS","SLOW_SPECTRUM",clLime);
  ini->WriteInteger("COLORS","SLOW_SPECTRUM",SlowSpectrumColor);
  BackColor= ini->ReadInteger("COLORS","BACKGROUND",clBlack);
  ini->WriteInteger("COLORS","BACKGROUND",BackColor);

  UploadExecStr=ini->ReadString("UPLOAD","EXEC_STRING","upload.bat");
  ini->WriteString("UPLOAD","EXEC_STRING",UploadExecStr);

  Upload1->Checked=ini->ReadBool("UPLOAD","ENABLE",false);
  ini->WriteBool("UPLOAD","ENABLE",Upload1->Checked);

  SHOW_HOUR=ini->ReadBool("WATERFALL","STACKING",false);
  ini->WriteBool("WATERFALL","STACKING",SHOW_HOUR);

 delete ini;

  Image1->Picture->Bitmap->PixelFormat = pf24bit;
  Image1->Picture->Bitmap->Height=NF;
  Image1->Picture->Bitmap->Width=LEN_WF+SPECTRUM_W;

  Flow=F0-Fspan/2.0;
  Fhigh=F0+Fspan/2.0;

  user2->Visible=false;
  int dial_menu=Fdial;
  switch(dial_menu)
  { case 135000:  N1350002->Checked=true; break;
    case 135500:  N1355002->Checked=true; break;
    case 136000:  N1360001->Checked=true; break;
    case 137000:  N1370001->Checked=true; break;
    case 125000:  N1250001->Checked=true; break;
    default:
     AnsiString sd; sd.sprintf("%.1f", Fdial);
     user2->Caption=sd; user2->Visible=true; user2->Checked=true;
  }

 userperiod1->Visible=false;
 switch(CAPTURE_PERIOD)
 { case 0: disabled1->Checked=true; break;
   case 1: N1min1->Checked=true; break;
   case 3: N3min1->Checked=true; break;
   case 5: N5min1->Checked=true; break;
   case 10: N10min1->Checked=true; break;
   case 15: N15min1->Checked=true; break;
   case 20: N20min1->Checked=true; break;
   case 30: N30min1->Checked=true; break;
   case 60: N1hour1->Checked=true; break;
   default: userperiod1->Caption=AnsiString(CAPTURE_PERIOD);
            userperiod1->Checked=true;userperiod1->Visible=true;
 }
 
 enable1->Checked=NB_ENABLE;
 int nb_menu=NB_THRESHOLD;
 switch(nb_menu)
 { case 3: N3dB1->Checked=true; break;
   case 6: N6dB1->Checked=true; break;
   case 10: N10dB1->Checked=true; break;
   case 15: N15dB1->Checked=true; break;
   case 20: N20dB1->Checked=true; break;
   case 30: N30dB1->Checked=true; break;
   default: user1->Caption=AnsiString(NB_THRESHOLD);
            user1->Checked=true;user1->Visible=true;
 }

 WAVEINCAPS pwic;
 int n=waveInGetNumDevs();
 for(unsigned int i=0; i<n; i++)
 { waveInGetDevCaps(i,&pwic,sizeof(pwic));
   AnsiString DevName=pwic.szPname;
   switch(i)
   {case 0: device01->Caption=DevName; device01->Visible=true; if(i==sndIn)device01->Checked=true;break;
    case 1: device11->Caption=DevName; device11->Visible=true; if(i==sndIn)device11->Checked=true;break;
    case 2: device21->Caption=DevName; device21->Visible=true; if(i==sndIn)device21->Checked=true;break;
    case 3: device31->Caption=DevName; device31->Visible=true; if(i==sndIn)device31->Checked=true;break;
    case 4: device41->Caption=DevName; device41->Visible=true; if(i==sndIn)device41->Checked=true;break;
    case 5: device51->Caption=DevName; device51->Visible=true; if(i==sndIn)device51->Checked=true;break;
    case 6: device61->Caption=DevName; device61->Visible=true; if(i==sndIn)device61->Checked=true;break;
    case 7: device71->Caption=DevName; device71->Visible=true; if(i==sndIn)device71->Checked=true;break;
    case 8: device81->Caption=DevName; device81->Visible=true; if(i==sndIn)device81->Checked=true;break;
   }
 }
  if(sndIn!=-1)
  {waveInGetDevCaps(sndIn,&pwic,sizeof(pwic));
   AnsiString DevName=pwic.szPname;
   StatusBar1->Panels->Items[3]->Text=DevName;
  }
  else StatusBar1->Panels->Items[3]->Text="default";
  snd=new TSound();
  snd->IN_DEVICE=sndIn;
  snd->openIn(SND_FD,16,1,SND_LEN);
  snd->openOut(SND_FD,16,1,SND_LEN);

  switch(CalibMenu)
  { case 0: autoDCFHGA1->Checked=true; break;
    case 1: fixedvalue1->Checked=true; break;
  }

  switch(CAPTURE_FORMAT)
  { case 0: BMP1->Click(); break;
    case 1: JPG1->Click(); break;
    default: BMP1->Click();
  }

  if(SHOW_HOUR) withStacking1->Checked=true; else Simple1->Checked=true;
  
  wf=new TWaveFile();
  rec=new TWaveFile();
  
  changeF(Fdial, F0);

  if(fabs(Fdial+F0-F_TA)<10) TA1->Checked=true;
  else if(fabs(Fdial+F0-F_EU)<10) EU1->Checked=true;
  else if(fabs(Fdial+F0-F_HGA22)<10) HGA221->Checked=true;
  else if(fabs(Fdial+F0-F_DCF39)<10) DCF391->Checked=true;
  else {F0_user=F0; AnsiString sf; sf.sprintf("%.1f", F0_user); user3->Caption=sf; user3->Checked=true; user3->Visible=true;}

  Image1->Canvas->Brush->Color= clGray;
  Image1->Canvas->Pen->Color= clGray;
  Image1->Canvas->FloodFill(1,1,clGray,fsBorder);

  AnsiString s; s.sprintf("Dial=%.1f",Fdial);
  Form1->StatusBar1->Panels->Items[0]->Text=s;
  s.sprintf("Fc=%.1f",F0);
  Form1->StatusBar1->Panels->Items[1]->Text=s;

  AnsiString sfv; sfv.sprintf("constant dF=%.3f",CalibratedDF);
  Form1->fixedvalue1->Caption=sfv;

  TrackBar1->Position=CONTRAST*100.0;

  DWORD RxThreadId;
  AUDIO_RX_WORKED=false;
  hAudioRx=CreateThread(NULL,0,AudioRxThread,0,CREATE_SUSPENDED,&RxThreadId);
  SetThreadPriority(hAudioRx, THREAD_PRIORITY_NORMAL);
  if(!AUDIO_RX_WORKED){ResumeThread(hAudioRx);}

 X1=X2=Y1=Y2=0;

 if(mycall=="" || myqth=="")
 {Label2->Caption="CALL:"; Label2->Visible=true;
  Edit2->Visible=true;
  Label4->Caption="QTH:"; Label4->Visible=true;
  Edit3->Visible=true;
  SpeedButton4->Visible=true;
 }

 int x1=paintSpectrum(SPECTRUM_W);
 int x2=paintFreqScale(x1);
 paintSpectrogram(x2);
 X_SCALE=x2;
}
//---------------------------------------------------------------------------



void __fastcall TForm1::device01Click(TObject *Sender)
{
 TIniFile *ini;
 ini = new TIniFile(ChangeFileExt( Application->ExeName, ".INI" ));
 ini->WriteInteger("SOUND","DEVICE",0);
 delete ini;
 snd->closeIn(); snd->closeOut();
 snd->IN_DEVICE=0;
 snd->openIn(SND_FD,16,1,SND_LEN);
 snd->openOut(SND_FD,16,1,SND_LEN);
 device01->Checked=true;
 StatusBar1->Panels->Items[3]->Text=device01->Caption;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::device11Click(TObject *Sender)
{
 TIniFile *ini;
 ini = new TIniFile(ChangeFileExt( Application->ExeName, ".INI" ));
 ini->WriteInteger("SOUND","DEVICE",1);
 delete ini;
 snd->closeIn(); snd->closeOut();
 snd->IN_DEVICE=1;
 snd->openIn(SND_FD,16,1,SND_LEN);
 snd->openOut(SND_FD,16,1,SND_LEN);
 device11->Checked=true;
 StatusBar1->Panels->Items[3]->Text=device11->Caption;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::device21Click(TObject *Sender)
{
 TIniFile *ini;
 ini = new TIniFile(ChangeFileExt( Application->ExeName, ".INI" ));
 ini->WriteInteger("SOUND","DEVICE",2);
 delete ini;
 snd->closeIn(); snd->closeOut();
 snd->IN_DEVICE=2;
 snd->openIn(SND_FD,16,1,SND_LEN);
 snd->openOut(SND_FD,16,1,SND_LEN);
 device21->Checked=true;
 StatusBar1->Panels->Items[3]->Text=device21->Caption;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::device31Click(TObject *Sender)
{
 TIniFile *ini;
 ini = new TIniFile(ChangeFileExt( Application->ExeName, ".INI" ));
 ini->WriteInteger("SOUND","DEVICE",3);
 delete ini;
 snd->closeIn(); snd->closeOut();
 snd->IN_DEVICE=3;
 snd->openIn(SND_FD,16,1,SND_LEN);
 snd->openOut(SND_FD,16,1,SND_LEN);
 device31->Checked=true;
 StatusBar1->Panels->Items[3]->Text=device31->Caption;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::device41Click(TObject *Sender)
{
  TIniFile *ini;
 ini = new TIniFile(ChangeFileExt( Application->ExeName, ".INI" ));
 ini->WriteInteger("SOUND","DEVICE",4);
 delete ini;
 snd->closeIn(); snd->closeOut();
 snd->IN_DEVICE=4;
 snd->openIn(SND_FD,16,1,SND_LEN);
 snd->openOut(SND_FD,16,1,SND_LEN);
 device41->Checked=true;
 StatusBar1->Panels->Items[3]->Text=device41->Caption;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::device51Click(TObject *Sender)
{
  TIniFile *ini;
 ini = new TIniFile(ChangeFileExt( Application->ExeName, ".INI" ));
 ini->WriteInteger("SOUND","DEVICE",5);
 delete ini;
 snd->closeIn(); snd->closeOut();
 snd->IN_DEVICE=5;
 snd->openIn(SND_FD,16,1,SND_LEN);
 snd->openOut(SND_FD,16,1,SND_LEN);
 device51->Checked=true;
 StatusBar1->Panels->Items[3]->Text=device51->Caption;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::device61Click(TObject *Sender)
{
 TIniFile *ini;
 ini = new TIniFile(ChangeFileExt( Application->ExeName, ".INI" ));
 ini->WriteInteger("SOUND","DEVICE",6);
 delete ini;
 snd->closeIn(); snd->closeOut();
 snd->IN_DEVICE=6;
 snd->openIn(SND_FD,16,1,SND_LEN);
 snd->openOut(SND_FD,16,1,SND_LEN);
 device61->Checked=true;
 StatusBar1->Panels->Items[3]->Text=device61->Caption;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::device71Click(TObject *Sender)
{
   TIniFile *ini;
 ini = new TIniFile(ChangeFileExt( Application->ExeName, ".INI" ));
 ini->WriteInteger("SOUND","DEVICE",7);
 delete ini;
 snd->closeIn(); snd->closeOut();
 snd->IN_DEVICE=7;
 snd->openIn(SND_FD,16,1,SND_LEN);
 snd->openOut(SND_FD,16,1,SND_LEN);
 device71->Checked=true;
 StatusBar1->Panels->Items[3]->Text=device71->Caption;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::device81Click(TObject *Sender)
{
 TIniFile *ini;
 ini = new TIniFile(ChangeFileExt( Application->ExeName, ".INI" ));
 ini->WriteInteger("SOUND","DEVICE",8);
 delete ini;
 snd->closeIn(); snd->closeOut();
 snd->IN_DEVICE=8;
 snd->openIn(SND_FD,16,1,SND_LEN);
 snd->openOut(SND_FD,16,1,SND_LEN);
 device81->Checked=true;
 StatusBar1->Panels->Items[3]->Text=device81->Caption;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::default1Click(TObject *Sender)
{
 TIniFile *ini;
 ini = new TIniFile(ChangeFileExt( Application->ExeName, ".INI" ));
 ini->WriteInteger("SOUND","DEVICE",-1);
 delete ini;
 snd->closeIn(); snd->closeOut();
 snd->IN_DEVICE=-1;
 snd->openIn(SND_FD,16,1,SND_LEN);
 snd->openOut(SND_FD,16,1,SND_LEN);
 default1->Checked=true;
 StatusBar1->Panels->Items[3]->Text=default1->Caption;       
}
//---------------------------------------------------------------------------


int TForm1::paintSpectrum(int width)
{
 int x0=Image1->Width - width;
 //cреднее значение - опорный уровень
 float Uslowsredn=0;
 for(int n=0; n<NF; n++) Uslowsredn+=Uslow[n];
 Uslowsredn=Uslowsredn/NF;
 float db0=-60;
 if(Uslowsredn>0 && Usred>0) db0=10.0*log10(Uslowsredn/Usred)+dbOffset;
 //округлить к десяткам
 db0=10.0*floor(db0/10.0);  // - опорный уровень, нижняя граница сетки дБ
                            //   (верхняя граница db0+40 дБ)
 //стереть
  Image1->Canvas->Brush->Color= BackColor;
  Image1->Canvas->Pen->Color= BackColor;
  Image1->Canvas->FillRect(Rect(x0,0,Image1->Width,Image1->Height));
 //сетка
  for(float f=floor(Flow); f<Fhigh; f=f+0.01)
  { int y= Image1->Height - ((f-Flow) * Image1->Height/Fspan);
    if(USE_LSB)y= ((f-Flow) * Image1->Height/Fspan);
    float df=fabs(f-floor(f));
     if(df<0.01)
     {Image1->Canvas->Brush->Color= BackColor;
      Image1->Canvas->Pen->Color= clDkGray;
      Image1->Canvas->Pen->Style = psDot;
      Image1->Canvas->MoveTo(Image1->Width,y);
      Image1->Canvas->LineTo(x0,y);
      Image1->Canvas->Pen->Style = psSolid;
     }
  }
 //спектр
  Image1->Canvas->Brush->Color= SpectrumColor;
  Image1->Canvas->Pen->Color= SpectrumColor;
  Image1->Canvas->MoveTo(x0, Image1->Height);
  if(USE_LSB)Image1->Canvas->MoveTo(x0, 0);
  for(int n=0; n<NF; n++)
  {//int i=Image1->Height * (NF-n) /NF;
    int y=Image1->Height * (NF-n-1) /NF;
    if(USE_LSB)y=Image1->Height * (n) /NF;  // перевернутый спектр
   if(U[n]>0 && Usred>0)
   {float u=10.0*log10(U[n]/Usred);
    float h1=width*(u - db0)/40.0;
    if(h1<0) h1=0;
    Image1->Canvas->LineTo(x0+h1,y);  //i
   }
  }

 //медленный спектр

  Image1->Canvas->Brush->Color= SlowSpectrumColor;
  Image1->Canvas->Pen->Color= SlowSpectrumColor;
  Image1->Canvas->MoveTo(x0, Image1->Height);
  if(USE_LSB)Image1->Canvas->MoveTo(x0, 0);

  for(int n=0; n<NF; n++)
  {//int i=Image1->Height * (NF-n) /NF;
    int y=Image1->Height * (NF-n-1) /NF;
    if(USE_LSB)y=Image1->Height * (n) /NF;  // перевернутый спектр
   if(Uslow[n]>0 && Usred>0)
   {float u=10.0*log10(Uslow[n]/Usred);
    float h1=width*(u - db0)/40.0;
    if(h1<0) h1=0;
    Image1->Canvas->LineTo(x0+h1,y); //i
   }
  }

   rotateText(270);
   //сетка децибелл
   for(int x=x0; x<Image1->Width; x+=width/4)
   {  Image1->Canvas->Brush->Color= BackColor;
      Image1->Canvas->Pen->Color= clDkGray;
      Image1->Canvas->Pen->Style = psDot;
      Image1->Canvas->MoveTo(x,0);
      Image1->Canvas->LineTo(x,Image1->Height);
      Image1->Canvas->Pen->Style = psSolid;
      Image1->Canvas->Font->Color=clDkGray;
      //Image1->Canvas->Brush->Style = bsClear;

      AnsiString s; s.sprintf("%.0fdB", db0+40.0*(x-x0)/width);
      int tw=Image1->Canvas->TextWidth(s);
      int th=Image1->Canvas->TextHeight(s);
      Image1->Canvas->TextOut(x+th+1,Image1->Height-tw-1,s);
      Image1->Canvas->Brush->Style = bsSolid;
   }
  return x0;
}

int TForm1::paintFreqScale(int x_spectrum_start)
{ AnsiString s1; s1.sprintf("%.0f", F0+Fdial);
  int width=Image1->Canvas->TextWidth(s1);
  int x0=x_spectrum_start - width-10;
  //стереть
  Image1->Canvas->Brush->Color= clWhite;
  Image1->Canvas->Pen->Color= clWhite;
  Image1->Canvas->FillRect(Rect(x0+1,0,x_spectrum_start-1,Image1->Height));
  rotateText(0);
  for(float f=floor(Flow); f<Fhigh; f=f+0.25)
  { int y= Image1->Height - ((f-Flow) * Image1->Height/Fspan);
    if(USE_LSB)y= ((f-Flow) * Image1->Height/Fspan);
    AnsiString s; s.sprintf("%.0f", f+Fdial);
    if(USE_LSB)s.sprintf("%.0f", Fdial-f);
    float df=fabs(f-floor(f));
    if((df>0.49&&df<0.51) || (df>0.24&&df<0.26) || (df>0.74&&df<0.76))
       {Image1->Canvas->Brush->Color= clWhite;
        Image1->Canvas->Pen->Color= clBlack;
        Image1->Canvas->Pen->Style = psSolid;
        if(df>0.49&&df<0.51) Image1->Canvas->MoveTo(x0+10,y);
        else Image1->Canvas->MoveTo(x0+5,y);
        Image1->Canvas->LineTo(x0,y);
       }
    if(df<0.01)
       {Image1->Canvas->Brush->Color= clWhite;
        Image1->Canvas->Pen->Color= clBlack;
        Image1->Canvas->Pen->Style = psSolid;
        Image1->Canvas->MoveTo(x0+width+9,y);
        //Image1->Canvas->MoveTo(Image1->Width,y);
        Image1->Canvas->LineTo(x0,y);
        Image1->Canvas->Pen->Style = psSolid;
        int tw=Image1->Canvas->TextWidth(s);
        int th=Image1->Canvas->TextHeight(s);
        Image1->Canvas->Brush->Color= clWhite;
        Image1->Canvas->Pen->Color= clWhite;
        Image1->Canvas->FillRect(Rect(x0+5,y-th/2,x0+6+tw,y+th/2));
        Image1->Canvas->Font->Color=clBlack;
        Image1->Canvas->Brush->Style = bsClear;
        Image1->Canvas->TextOut(x0+5,y-th/2,s);
        Image1->Canvas->Brush->Style = bsSolid;
       }
  }

  return x0;
}

TForm1::paintSpectrogram(int x_start)
{
  //цветовая шкала
  for(int i=0; i<Image2->Width; i++)
  { float yy=1.0*i/Image2->Width;
     yy=yy*CONTRAST;
     int R=408*yy*sqrt(yy) - 153;     if(R<0) R=0; if(R>255) R=255;
     int G=408*yy*sqrt(yy) - 153;     if(G<0) G=0; if(G>255) G=255;
     int B=448*yy +32; if(yy>0.90) B=2550*(1.0-yy); if(B<0) B=0; if(B>255) B=255;
     Image2->Canvas->Brush->Color= clBlack;
     Image2->Canvas->Pen->Color= RGB(R,G,B);
     Image2->Canvas->MoveTo(i,0);
     Image2->Canvas->LineTo(i,Image2->Height);
  }
  //отметка dbOffset на шкале
  Image2->Canvas->Brush->Color= clBlack;
  Image2->Canvas->Pen->Color= clRed;
  int x_offset=-Image2->Width*dbOffset/40.0;
  Image2->Canvas->MoveTo(x_offset,0);
  Image2->Canvas->LineTo(x_offset,Image2->Height);
  
  rotateText(270);
 //отобразить спектрограмму
 //Image1->Visible=false;
 for(int n=0; n<NF; n++)
 {int y_old=0;
  float yy_old=0;
  int y=Image1->Height * (NF-n-1) /NF;
  if(USE_LSB)y=Image1->Height * (n) /NF;  // перевернутый спектр
  Byte *ptr  = (Byte *)Image1->Picture->Bitmap->ScanLine[y];
  int cnt=0;
  for(int j=0; j<x_start; j++)
  {float yy=0;
   if(WF[n][j]>0 && WFsred[j]>0)
   {float u=10.0*log10(WF[n][j]/WFsred[j])-dbOffset;
    yy=u/40.0; //здесь контраст, вся шкала 40
   }
   if(SHOW_HOUR && j<LEN_HOUR)
   { if(WF_HOUR[n][j]>0 && WFHsred[j]>0) //показывать с накоплением
    {float u=10.0*log10(WF_HOUR[n][j]/WFHsred[j])-dbOffset;
     yy=u/40.0; //здесь контраст, вся шкала 40
    }
   }
   yy=yy*CONTRAST;
     if(yy<0)yy=0; if(yy>1)yy=1;
     int R=408*yy - 153; if(R<0) R=0; if(R>255) R=255; //-153
     int G=408*yy - 153; if(G<0) G=0; if(G>255) G=255;
     int B=448*yy + 32; if(yy>0.90) B=2550*(1.0-yy); if(B<0) B=0; if(B>255) B=255;
       ptr[3*(x_start-j)]=(Byte)B;
       ptr[3*(x_start-j)+1]=(Byte)G;
       ptr[3*(x_start-j)+2]=(Byte)R;
  }
 }
  //Image1->Visible=true;
  //отметка зоны накопления
   if(SHOW_HOUR)
   {
    Image1->Canvas->Brush->Color=clRed;
    Image1->Canvas->Pen->Color= clRed;
    Image1->Canvas->Pen->Style = psDot;
    Image1->Canvas->Brush->Style = psDot;
    Image1->Canvas->MoveTo(x_start,1);
    Image1->Canvas->LineTo(x_start-LEN_HOUR,1);
    Image1->Canvas->LineTo(x_start-LEN_HOUR,Image1->Height-1);
   }
   Image1->Canvas->Pen->Style = psSolid;
   Image1->Canvas->Brush->Style = bsSolid;
 // отметки времени на водопаде
  time_t t_label=time(NULL);
  time_t t1;
  struct tm *t_cur, *t_x;
  static int old_min=-1;
  t_cur=gmtime(&t_label);
  for(int j=0; j<x_start; j++)
  { t1=t_label-j*10;
    t_x=gmtime(&t1);
    if(t_x->tm_min%20==0 && t_x->tm_min!= old_min)
    { old_min=t_x->tm_min;
      AnsiString label_time;
      label_time.sprintf("%02u:%02u",t_x->tm_hour,t_x->tm_min);
      int tw=Image1->Canvas->TextWidth(label_time);
      int th=Image1->Canvas->TextHeight(label_time);
      Image1->Canvas->Pen->Color= clWhite;
      Image1->Canvas->MoveTo(x_start-j,Image1->Height);
      if(t_x->tm_min==0)Image1->Canvas->LineTo(x_start-j,Image1->Height-tw-5);
      else Image1->Canvas->LineTo(x_start-j,Image1->Height-tw-2);
      Image1->Canvas->Font->Color=clWhite;
      //Image1->Canvas->Brush->Color=clWhite;
      //Image1->Canvas->Pen->Color=clWhite;
      Image1->Canvas->Brush->Style = bsClear;
      Image1->Canvas->TextOut(x_start-j+th+1,Image1->Height-tw-1,label_time);
      Image1->Canvas->Brush->Style = bsSolid;
    }
  }

  // штамп с информацией
  {
   time_t t0=time(NULL);
   struct tm *tt;
   tt=gmtime(&t0);
   rotateText(0);
   Image1->Canvas->Brush->Color=clBlack;
   Image1->Canvas->Pen->Color= clBlack;
   Image1->Canvas->Pen->Style = psSolid;
   //Image1->Canvas->Brush->Style = bsClear;
   Image1->Canvas->Font->Color=clWhite;
   Image1->Canvas->Font->Style = TFontStyles()<< fsBold;

   AnsiString s0;
   s0.sprintf(" %s %s %02d-%02d-%04d %02d:%02d UTC ", mycall,myqth, tt->tm_mday,tt->tm_mon+1,tt->tm_year+1900, tt->tm_hour,tt->tm_min);
   if(SHOW_HOUR) s0=s0+"(stacking) ";
   int tw=Image1->Canvas->TextWidth(s0);
   int th=Image1->Canvas->TextHeight(s0);
   Image1->Canvas->TextOut(1,1,s0);
   Image1->Canvas->Font->Style = TFontStyles();
  }

  //рамка вокруг всей картинки
   Image1->Canvas->Brush->Color=clBlack;
   Image1->Canvas->Pen->Color= clBlack;
   Image1->Canvas->Pen->Style = psSolid;
   Image1->Canvas->MoveTo(0,0);
   Image1->Canvas->LineTo(Image1->Width-1,0);
   Image1->Canvas->LineTo(Image1->Width-1,Image1->Height-1);
   Image1->Canvas->LineTo(0,Image1->Height-1);
   Image1->Canvas->LineTo(0,0);

}
//////////////////////////////////////////////////////////////////////////////////

void __fastcall TForm1::Timer1Timer(TObject *Sender)
{int x1=paintSpectrum(SPECTRUM_W);
 int x2=paintFreqScale(x1);
}
//---------------------------------------------------------------------------


TForm1::rotateText(int angle)
{
    LOGFONT lf; // Windows native font structure

    Image1->Canvas->Brush->Style = bsClear; // set the brush style to transparent
    ZeroMemory(&lf, sizeof(LOGFONT));

    lf.lfHeight = 13;
    lf.lfEscapement = 10 * angle; // degrees to rotate
    lf.lfOrientation = 10 * angle;
    lf.lfCharSet = DEFAULT_CHARSET;
    strcpy(lf.lfFaceName, "Tahoma");
    Image1->Canvas->Font->Handle = CreateFontIndirect(&lf);

}

void __fastcall TForm1::Timer2Timer(TObject *Sender)
{
 int x1=paintSpectrum(SPECTRUM_W);
 int x2=paintFreqScale(x1);
 paintSpectrogram(x2);

 //сохранять на диск раз во столько-то минут
  if(CAPTURE_PERIOD>0)
  { time_t t=time(NULL);
    struct tm *t_cur;
    static int old_min_c=-1;
    t_cur=gmtime(&t);
    if(t_cur->tm_min%CAPTURE_PERIOD==0 && t_cur->tm_min!= old_min_c)
    { old_min_c=t_cur->tm_min;
      if(CAPTURE_FORMAT==0) // bmp
      {
       Image1->Picture->SaveToFile(CaptureFilename);
      }
      if(CAPTURE_FORMAT==1) // jpg
      {TJPEGImage *jp = new TJPEGImage();
       jp->Smoothing=false;
       jp->CompressionQuality=100;
       try
       { jp->Assign(Image1->Picture->Bitmap);
         jp->SaveToFile(CaptureFilename);
       }
      __finally { delete jp; }
      }
      AnsiString s; s.sprintf("%s saved at %02d:%02d:%02d",CaptureFilename,t_cur->tm_hour,t_cur->tm_min,t_cur->tm_sec);
      StatusBar1->Panels->Items[6]->Text=s;
      if(Upload1->Checked)
      { AnsiString runstr=UploadExecStr + " " +CaptureFilename;
        int result=WinExec(runstr.c_str(),SW_SHOWMINNOACTIVE);
        if(result>31)StatusBar1->Panels->Items[6]->Text=s+" and uploaded";
        else  StatusBar1->Panels->Items[6]->Text=s+", upload error!";
      }
    }
  }
  
}
//---------------------------------------------------------------------------

void __fastcall TForm1::disabled1Click(TObject *Sender)
{
 CAPTURE_PERIOD=0;
 TIniFile *ini;
 ini = new TIniFile(ChangeFileExt( Application->ExeName, ".INI" ));
 ini->WriteInteger("GRABBER","CAPTURE_PERIOD",CAPTURE_PERIOD);
 delete ini;
 disabled1->Checked=true;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::N5min1Click(TObject *Sender)
{
 CAPTURE_PERIOD=5;
 TIniFile *ini;
 ini = new TIniFile(ChangeFileExt( Application->ExeName, ".INI" ));
 ini->WriteInteger("GRABBER","CAPTURE_PERIOD",CAPTURE_PERIOD);
 delete ini;
 N5min1->Checked=true;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::N10min1Click(TObject *Sender)
{
 CAPTURE_PERIOD=10;
 TIniFile *ini;
 ini = new TIniFile(ChangeFileExt( Application->ExeName, ".INI" ));
 ini->WriteInteger("GRABBER","CAPTURE_PERIOD",CAPTURE_PERIOD);
 delete ini;
  N10min1->Checked=true;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::N15min1Click(TObject *Sender)
{
 CAPTURE_PERIOD=15;
 TIniFile *ini;
 ini = new TIniFile(ChangeFileExt( Application->ExeName, ".INI" ));
 ini->WriteInteger("GRABBER","CAPTURE_PERIOD",CAPTURE_PERIOD);
 delete ini;
  N15min1->Checked=true;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::N20min1Click(TObject *Sender)
{
 CAPTURE_PERIOD=20;
 TIniFile *ini;
 ini = new TIniFile(ChangeFileExt( Application->ExeName, ".INI" ));
 ini->WriteInteger("GRABBER","CAPTURE_PERIOD",CAPTURE_PERIOD);
 delete ini;
  N20min1->Checked=true;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::N30min1Click(TObject *Sender)
{
 CAPTURE_PERIOD=30;
 TIniFile *ini;
 ini = new TIniFile(ChangeFileExt( Application->ExeName, ".INI" ));
 ini->WriteInteger("GRABBER","CAPTURE_PERIOD",CAPTURE_PERIOD);
 delete ini;
  N30min1->Checked=true;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::userperiod1Click(TObject *Sender)
{
 CAPTURE_PERIOD = userperiod1->Caption.ToInt();
 TIniFile *ini;
 ini = new TIniFile(ChangeFileExt( Application->ExeName, ".INI" ));
 ini->WriteInteger("GRABBER","CAPTURE_PERIOD",CAPTURE_PERIOD);
 delete ini;
 userperiod1->Checked=true;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------

void __fastcall TForm1::Image2MouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
 dbOffset=-40.0*X/Image2->Width;
 int x1=paintSpectrum(80);
 int x2=paintFreqScale(x1);
 paintSpectrogram(x2);
 TIniFile *ini;
 ini = new TIniFile(ChangeFileExt( Application->ExeName, ".INI" ));
 ini->WriteFloat("GRABBER","DB_OFFSET",dbOffset);
 delete ini;
}
//---------------------------------------------------------------------------


void __fastcall TForm1::N1hour1Click(TObject *Sender)
{
 CAPTURE_PERIOD=60;
 TIniFile *ini;
 ini = new TIniFile(ChangeFileExt( Application->ExeName, ".INI" ));
 ini->WriteInteger("GRABBER","CAPTURE_PERIOD",CAPTURE_PERIOD);
 delete ini;
  N1hour1->Checked=true;
}
//---------------------------------------------------------------------------


void __fastcall TForm1::Openwav1Click(TObject *Sender)
{
//
 OpenDialog1->FileName="";
 OpenDialog1->Execute();
 AnsiString wavefile=OpenDialog1->FileName;
 if(wavefile=="")return;
 wf->open(wavefile,"r");
 int sps=SND_FD;
 if(F0>=5450) sps=SND_FD2;
 if(wf->Format.nSamplesPerSec!=sps || wf->Format.wBitsPerSample!=16)
 {
  AnsiString s="WAV-file must be "+AnsiString(sps)+" sps, 16 bit!";
  MessageBox(NULL,s.c_str(),"WAV error",MB_OK|MB_ICONWARNING);
  processWav=false;
  return;
 }
 StatusBar1->Panels->Items[6]->Text="Processing "+ExtractFileName(wavefile);
 processWav=true;
 snd->closeIn();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::FormClose(TObject *Sender, TCloseAction &Action)
{
 if(AUDIO_RX_WORKED){SuspendThread(hAudioRx);TerminateThread(hAudioRx,0);}
  TIniFile *ini;
  ini = new TIniFile(ChangeFileExt( Application->ExeName, ".INI" ));
  ini->WriteString("GRABBER","MYCALL",mycall);
  ini->WriteString("GRABBER","MYQTH",myqth);
  ini->WriteFloat("GRABBER" ,"F_CENTER",F0);
  ini->WriteFloat("GRABBER" ,"F_DIAL",Fdial);
  ini->WriteInteger("GRABBER" ,"CALIBRATION",CalibMenu);
  ini->WriteFloat("GRABBER" ,"CALIBRATED_DF",CalibratedDF);
  ini->WriteInteger("GRABBER","CAPTURE_PERIOD",CAPTURE_PERIOD);
  ini->WriteInteger("GRABBER","CAPTURE_FORMAT",CAPTURE_FORMAT);
  ini->WriteFloat("GRABBER" ,"DB_OFFSET",dbOffset);
  ini->WriteFloat("GRABBER" ,"CONTRAST",CONTRAST);
  ini->WriteInteger("GRABBER" ,"SPECTRUM_WIDTH",SPECTRUM_W);
  ini->WriteString("GRABBER" ,"CAPTURE_FILENAME",CaptureFilename);
  ini->WriteBool("GRABBER","NB_ENABLE",NB_ENABLE);
  ini->WriteFloat("GRABBER","NB_THRESHOLD",NB_THRESHOLD);
  ini->WriteBool("UPLOAD","ENABLE",Upload1->Checked);

  delete ini;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::autoDCFHGA1Click(TObject *Sender)
{
  autoDCFHGA1->Checked=true;
  CalibMenu=0;
  TIniFile *ini;
  ini = new TIniFile(ChangeFileExt( Application->ExeName, ".INI" ));
  ini->WriteInteger("GRABBER" ,"CALIBRATION",CalibMenu);
  delete ini;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::fixedvalue1Click(TObject *Sender)
{
  //
  fixedvalue1->Checked=true;
  CalibMenu=1;
  TIniFile *ini;
  ini = new TIniFile(ChangeFileExt( Application->ExeName, ".INI" ));
  ini->WriteInteger("GRABBER" ,"CALIBRATION",CalibMenu);
  ini->WriteFloat("GRABBER","CALIBRATED_DF",CalibratedDF);
  delete ini;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::TrackBar1Change(TObject *Sender)
{
 CONTRAST=TrackBar1->Position/100.0;
 AnsiString s; s.sprintf("Contrast=%.1f%%",CONTRAST*100);
 TrackBar1->Hint=s;
 int x1=paintSpectrum(80);
 int x2=paintFreqScale(x1);
 paintSpectrogram(x2);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::SpeedButton2Click(TObject *Sender)
{
 time_t t=time(NULL);
 struct tm *t_cur;
 t_cur=gmtime(&t);

 AnsiString cs;
 if(CAPTURE_FORMAT==0) // bmp
 { cs.sprintf("%scapt_%04d-%02d-%02d_%02d%02d%02d.bmp",CapturePath,t_cur->tm_year+1900,t_cur->tm_mon+1,t_cur->tm_mday,t_cur->tm_hour,t_cur->tm_min,t_cur->tm_sec);
   Image1->Picture->SaveToFile(cs);
 }
 if(CAPTURE_FORMAT==1) // jpg
 {TJPEGImage *jp = new TJPEGImage();
  jp->Smoothing=false;
  jp->CompressionQuality=100;
  try
  { jp->Assign(Image1->Picture->Bitmap);
    cs.sprintf("%scapt_%04d-%02d-%02d_%02d%02d%02d.jpg",CapturePath,t_cur->tm_year+1900,t_cur->tm_mon+1,t_cur->tm_mday,t_cur->tm_hour,t_cur->tm_min,t_cur->tm_sec);
    jp->SaveToFile(cs);
  }
  __finally { delete jp; }
 }

 AnsiString s; s.sprintf("%s saved",cs);
 StatusBar1->Panels->Items[6]->Text=s;
}
//---------------------------------------------------------------------------


void __fastcall TForm1::enable1Click(TObject *Sender)
{
 if(enable1->Checked) enable1->Checked=false;
 else enable1->Checked=true;
 NB_ENABLE=enable1->Checked;
}
//---------------------------------------------------------------------------



void __fastcall TForm1::N6dB1Click(TObject *Sender)
{
  N6dB1->Checked=true;
  NB_THRESHOLD=6;
  CLIP_POROG=pow(10,NB_THRESHOLD/20.0);
  TIniFile *ini;
  ini = new TIniFile(ChangeFileExt( Application->ExeName, ".INI" ));
  ini->WriteFloat("GRABBER","NB_THRESHOLD",NB_THRESHOLD);
  delete ini;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::N10dB1Click(TObject *Sender)
{
  N10dB1->Checked=true;
  NB_THRESHOLD=10;
  CLIP_POROG=pow(10,NB_THRESHOLD/20.0);
  TIniFile *ini;
  ini = new TIniFile(ChangeFileExt( Application->ExeName, ".INI" ));
  ini->WriteFloat("GRABBER","NB_THRESHOLD",NB_THRESHOLD);
  delete ini;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::N15dB1Click(TObject *Sender)
{
  N15dB1->Checked=true;
  NB_THRESHOLD=15;
  CLIP_POROG=pow(10,NB_THRESHOLD/20.0);
  TIniFile *ini;
  ini = new TIniFile(ChangeFileExt( Application->ExeName, ".INI" ));
  ini->WriteFloat("GRABBER","NB_THRESHOLD",NB_THRESHOLD);
  delete ini;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::N20dB1Click(TObject *Sender)
{
  N20dB1->Checked=true;
  NB_THRESHOLD=20;
  CLIP_POROG=pow(10,NB_THRESHOLD/20.0);
  TIniFile *ini;
  ini = new TIniFile(ChangeFileExt( Application->ExeName, ".INI" ));
  ini->WriteFloat("GRABBER","NB_THRESHOLD",NB_THRESHOLD);
  delete ini;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::N30dB1Click(TObject *Sender)
{
  N30dB1->Checked=true;
  NB_THRESHOLD=30;
  CLIP_POROG=pow(10,NB_THRESHOLD/20.0);
  TIniFile *ini;
  ini = new TIniFile(ChangeFileExt( Application->ExeName, ".INI" ));
  ini->WriteFloat("GRABBER","NB_THRESHOLD",NB_THRESHOLD);
  delete ini;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::user1Click(TObject *Sender)
{
  user1->Checked=true;
  NB_THRESHOLD=user1->Caption.ToDouble();
  CLIP_POROG=pow(10,NB_THRESHOLD/20.0);
  TIniFile *ini;
  ini = new TIniFile(ChangeFileExt( Application->ExeName, ".INI" ));
  ini->WriteFloat("GRABBER","NB_THRESHOLD",NB_THRESHOLD);
  delete ini;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::N3dB1Click(TObject *Sender)
{
  N3dB1->Checked=true;
  NB_THRESHOLD=3;
  CLIP_POROG=pow(10,NB_THRESHOLD/20.0);
  TIniFile *ini;
  ini = new TIniFile(ChangeFileExt( Application->ExeName, ".INI" ));
  ini->WriteFloat("GRABBER","NB_THRESHOLD",NB_THRESHOLD);
  delete ini;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::N1min1Click(TObject *Sender)
{
 CAPTURE_PERIOD=1;
 TIniFile *ini;
 ini = new TIniFile(ChangeFileExt( Application->ExeName, ".INI" ));
 ini->WriteInteger("GRABBER","CAPTURE_PERIOD",CAPTURE_PERIOD);
 delete ini;
  N1min1->Checked=true;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::N3min1Click(TObject *Sender)
{
  CAPTURE_PERIOD=3;
 TIniFile *ini;
 ini = new TIniFile(ChangeFileExt( Application->ExeName, ".INI" ));
 ini->WriteInteger("GRABBER","CAPTURE_PERIOD",CAPTURE_PERIOD);
 delete ini;
  N3min1->Checked=true;
}
//---------------------------------------------------------------------------


void __fastcall TForm1::enterdF1Click(TObject *Sender)
{
 AnsiString s; s.sprintf("%.3f",CalibratedDF);
 Edit1->Text=s;
 Edit1->Visible=true;
 Label3->Caption="Offset [Hz]:";
 Label3->Visible=true;
 SpeedButton3->Visible=true;
 Edit1->SetFocus();
 OK_Button_mode=0;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::SpeedButton3Click(TObject *Sender)
{
 Edit1->Visible=false;
 Label3->Visible=false;
 SpeedButton3->Visible=false;
 try {
  if(OK_Button_mode==0)
  {CalibratedDF=Edit1->Text.ToDouble();
   AnsiString sfv; sfv.sprintf("constant dF=%.3f",CalibratedDF);
   Form1->fixedvalue1->Caption=sfv;
  }
  if(OK_Button_mode==1)
  {
    double Fdnew=Edit1->Text.ToDouble();
    changeF(Fdnew,Fdial+F0-Fdnew);
    AnsiString sd; sd.sprintf("%.1f", Fdnew);
    user2->Caption=sd; user2->Checked=true; user2->Visible=true;
  }
  if(OK_Button_mode==2)
  {
   user3->Caption=Edit1->Text;  user3->Checked=true; user3->Visible=true;
   F0_user=Edit1->Text.ToDouble();
   changeF(Fdial,F0_user);
  }
 }catch(...){;}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Edit1Change(TObject *Sender)
{int dotpos=0;
 if(DecimalSeparator==',')dotpos=Edit1->Text.Pos(".");
 if(DecimalSeparator=='.')dotpos=Edit1->Text.Pos(",");
 if(dotpos>0){
    AnsiString s1=Edit1->Text.SubString(1,dotpos-1);
    AnsiString s2=Edit1->Text.SubString(dotpos+1,Edit1->Text.Length()-dotpos);
    Edit1->Text=s1+DecimalSeparator+s2;
     Edit1->SetFocus();
 }
 try{
  double nbt=Edit1->Text.ToDouble();
  }
 catch(...){Edit1->Text=Edit1->Text.SubString(1,Edit1->Text.Length()-1);}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Edit1KeyPress(TObject *Sender, char &Key)
{
  if(Key=='\x1B') {
     Edit1->Visible=false;
     Label3->Visible=false;
     SpeedButton3->Visible=false;
  }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::BMP1Click(TObject *Sender)
{
 BMP1->Checked=true;
 CAPTURE_FORMAT=0;
 int pos=CaptureFilename.Pos(".jpg");
 if(pos>0){AnsiString s1=CaptureFilename.SubString(1,pos-1);
  CaptureFilename=s1+".bmp";
 }
 TIniFile *ini;
 ini = new TIniFile(ChangeFileExt( Application->ExeName, ".INI" ));
 ini->WriteString("GRABBER" ,"CAPTURE_FILENAME",CaptureFilename);
 ini->WriteInteger("GRABBER","CAPTURE_FORMAT",CAPTURE_FORMAT);
 delete ini;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::JPG1Click(TObject *Sender)
{
 JPG1->Checked=true;
 CAPTURE_FORMAT=1;
 int pos=CaptureFilename.Pos(".bmp");
 if(pos>0){AnsiString s1=CaptureFilename.SubString(1,pos-1);
  CaptureFilename=s1+".jpg";
 }
 TIniFile *ini;
 ini = new TIniFile(ChangeFileExt( Application->ExeName, ".INI" ));
 ini->WriteString("GRABBER" ,"CAPTURE_FILENAME",CaptureFilename);
 ini->WriteInteger("GRABBER","CAPTURE_FORMAT",CAPTURE_FORMAT);
 delete ini;
}
//---------------------------------------------------------------------------


void __fastcall TForm1::EnterFdial1Click(TObject *Sender)
{
 AnsiString s; s.sprintf("%.1f",Fdial);
 Edit1->Text=s;
 Edit1->Visible=true;
 Label3->Caption="Fdial [Hz]:";
 Label3->Visible=true;
 SpeedButton3->Visible=true;
 Edit1->SetFocus();
 OK_Button_mode=1;
}
//---------------------------------------------------------------------------


void TForm1::changeF(double Fd, double Fc)
{//настройки фильтров для выбранной Fdial и Fc
 static bool first_time=true;
 if(!first_time)
 { if(Fd==Fdial && Fc==F0) return;
   if(AUDIO_RX_WORKED){SuspendThread(hAudioRx);TerminateThread(hAudioRx,0);AUDIO_RX_WORKED=false;}
   delete BPF_F; delete BPF_F1; delete BPF_F11; delete BPF_F2;
   delete[] BPF; delete[] BPF2; delete[] BPF3;
   delete[] LPF;
   delete filter1;  delete filter11;  delete filter111;
 }
   Fdial=fabs(Fd); F0=fabs(Fc); if(Fc<0) USE_LSB=true; else USE_LSB=false;
   Fhigh=F0+Fspan/2; Flow=F0-Fspan/2;
   if(F0<5450) {Fget=F0-177.0;dph=TWOPI*(Fget)/SND_FD; dph2=0; }
   else {Fget=1777.0-177.0;dph=TWOPI*(Fget)/SND_FD; dph2=TWOPI*(F0-1777.0)/SND_FD2;}
  // для спектроанализатора
  // создание необходимого числа фильтров
  for(int n=0; n<NF; n++)
  {float f=(Flow-Fget) + n*((Fhigh-Flow)/NF); //частота фильтра
   BPF[n]=new TIIRBPF2(f/SND_FD*K12,(float)DF/SND_FD*K12);
    BPF2[n]=new TIIRBPF2(f/SND_FD*K12,(float)DF/SND_FD*K12);
    BPF3[n]=new TIIRBPF2(f/SND_FD*K12,(float)DF/SND_FD*K12);
   LPF[n]=new TIIRLPF(3.0/60.0/SND_FD*K12,1); //1.5 60 - длина посылки 60 сек
   memset(&WF[n][0],0,(LEN_WF)*sizeof(float));
   memset(&WF_HOUR[n][0],0,(LEN_HOUR)*sizeof(float));
  }

  // создание входного фильтра широкого
  if(F0<5450)
  {filter1=new TIIRBPF2((F0-50.0)/SND_FD,(F0+50.0)/SND_FD);
   filter11=new TIIRBPF2((F0-50.0)/SND_FD,(F0+50.0)/SND_FD);
   filter111=new TIIRBPF2((F0-50.0)/SND_FD,(F0+50.0)/SND_FD);
  }
  else { filter1=new TIIRBPF2((F0-50.0)/SND_FD2,(F0+50.0)/SND_FD2);
         filter11=new TIIRBPF2((F0-50.0)/SND_FD,(F0+50.0)/SND_FD);
         filter111=new TIIRBPF2((F0-50.0)/SND_FD,(F0+50.0)/SND_FD);
       }
 // для частотомера
  if(Fdial>135200)
  {BPF_F=new TIIRBPF2((F_DCF39-Fdial)/SND_FD, BW_F/SND_FD);
   BPF_F1=new TIIRBPF2((F_DCF39-Fdial)/SND_FD, BW_F/SND_FD);
   BPF_F11=new TIIRBPF2((F_DCF39-Fdial)/SND_FD, BW_F/SND_FD);
   BPF_F2=new TIIRBPF2((F_DCF39-Fdial)/SND_FD, BW_F/SND_FD);
  }
  else
  {BPF_F=new TIIRBPF2((F_HGA22-Fdial)/SND_FD, BW_F/SND_FD);
   BPF_F1=new TIIRBPF2((F_HGA22-Fdial)/SND_FD, BW_F/SND_FD);
   BPF_F11=new TIIRBPF2((F_HGA22-Fdial)/SND_FD, BW_F/SND_FD);
   BPF_F2=new TIIRBPF2((F_HGA22-Fdial)/SND_FD, BW_F*10.0/SND_FD);
  }

  memset(xf,0,sizeof(xf));memset(xf2,0,sizeof(xf2));
  memset(df0,0,sizeof(df0));

  if(!first_time)
  {
   DWORD RxThreadId;
   AUDIO_RX_WORKED=false;
   hAudioRx=CreateThread(NULL,0,AudioRxThread,0,CREATE_SUSPENDED,&RxThreadId);
   SetThreadPriority(hAudioRx, THREAD_PRIORITY_NORMAL);
   if(!AUDIO_RX_WORKED){ResumeThread(hAudioRx);}
  }
  first_time=false;
  AnsiString s; s.sprintf("Dial=%.1f",Fdial);
  Form1->StatusBar1->Panels->Items[0]->Text=s;
  s.sprintf("Fc=%.1f",F0);
  Form1->StatusBar1->Panels->Items[1]->Text=s;
  int x1=paintSpectrum(SPECTRUM_W);
  int x2=paintFreqScale(x1);
  paintSpectrogram(x2);
}


void __fastcall TForm1::EnterFc1Click(TObject *Sender)
{
 AnsiString s; s.sprintf("%.1f",F0);
 Edit1->Text=s;
 Edit1->Visible=true;
 Label3->Caption="Audio Fc:";
 Label3->Visible=true;
 SpeedButton3->Visible=true;
 Edit1->SetFocus();
 OK_Button_mode=2;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::TA1Click(TObject *Sender)
{
 TA1->Checked=true;
 changeF(Fdial, F_TA-Fdial);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::EU1Click(TObject *Sender)
{
 EU1->Checked=true;
 changeF(Fdial, F_EU-Fdial);
}
//---------------------------------------------------------------------------


void __fastcall TForm1::N1350002Click(TObject *Sender)
{
 N1350002->Checked=true;
 changeF(135000.0,Fdial+F0-135000.0);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::N1355002Click(TObject *Sender)
{
 N1355002->Checked=true;
 changeF(135500.0,Fdial+F0-135500.0);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::N1360001Click(TObject *Sender)
{
 N1360001->Checked=true;
 changeF(136000.0,Fdial+F0-136000.0);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::N1370001Click(TObject *Sender)
{
 N1370001->Checked=true;
 changeF(137000.0,Fdial+F0-137000.0);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::N1250001Click(TObject *Sender)
{
 N1250001->Checked=true;
 changeF(125000.0,Fdial+F0-125000.0);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::user3Click(TObject *Sender)
{
 user3->Checked=true;
 changeF(Fdial,F0_user);
}
//---------------------------------------------------------------------------


void __fastcall TForm1::HGA221Click(TObject *Sender)
{
  HGA221->Checked=true;
  changeF(Fdial, F_HGA22-Fdial);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::DCF391Click(TObject *Sender)
{
  DCF391->Checked=true;
  changeF(Fdial, F_DCF39-Fdial);
}
//---------------------------------------------------------------------------


void __fastcall TForm1::Spectrum1Click(TObject *Sender)
{
 ColorDialog1->Color=SpectrumColor;
 ColorDialog1->Execute();
 SpectrumColor=ColorDialog1->Color;
 TIniFile *ini;
 ini = new TIniFile(ChangeFileExt( Application->ExeName, ".INI" ));
   ini->WriteInteger("COLORS","SPECTRUM",SpectrumColor);
 delete ini;
  int x1=paintSpectrum(SPECTRUM_W);
  int x2=paintFreqScale(x1);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::SlowSpectrum1Click(TObject *Sender)
{
 ColorDialog1->Color=SlowSpectrumColor;
 ColorDialog1->Execute();
 SlowSpectrumColor=ColorDialog1->Color;
 TIniFile *ini;
 ini = new TIniFile(ChangeFileExt( Application->ExeName, ".INI" ));
   ini->WriteInteger("COLORS","SLOW_SPECTRUM",SlowSpectrumColor);
 delete ini;
  int x1=paintSpectrum(SPECTRUM_W);
  int x2=paintFreqScale(x1);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Backgfound1Click(TObject *Sender)
{
 ColorDialog1->Color=BackColor;
 ColorDialog1->Execute();
 BackColor=ColorDialog1->Color;
 TIniFile *ini;
 ini = new TIniFile(ChangeFileExt( Application->ExeName, ".INI" ));
   ini->WriteInteger("COLORS","BACKGROUND",BackColor);
 delete ini;
  int x1=paintSpectrum(SPECTRUM_W);
  int x2=paintFreqScale(x1);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::default2Click(TObject *Sender)
{
 SpectrumColor=clYellow;
 SlowSpectrumColor=clLime;
 BackColor=clBlack;
 TIniFile *ini;
 ini = new TIniFile(ChangeFileExt( Application->ExeName, ".INI" ));
   ini->WriteInteger("COLORS","SPECTRUM",SpectrumColor);
   ini->WriteInteger("COLORS","SLOW_SPECTRUM",SlowSpectrumColor);
   ini->WriteInteger("COLORS","BACKGROUND",BackColor);
 delete ini;
  int x1=paintSpectrum(SPECTRUM_W);
  int x2=paintFreqScale(x1);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::SpeedButton1Click(TObject *Sender)
{
 time_t t=time(NULL);
 struct tm *t_cur;
 t_cur=gmtime(&t);
 
 if(RECORDING)
 { SpeedButton1->Caption="Rec";
   RECORDING=false;
   Sleep(1000);
   rec->close();
   bytes_recorded=0;
   Form1->Label2->Visible=false;
 }
 else
 { SpeedButton1->Caption="Stop";
   Form1->Label2->Visible=true; Form1->Label2->Caption="record start...";
   bytes_recorded=0;
   recfile.sprintf("%srec_%04d-%02d-%02d_%02d%02d%02d.wav",CapturePath,t_cur->tm_year+1900,t_cur->tm_mon+1,t_cur->tm_mday,t_cur->tm_hour,t_cur->tm_min,t_cur->tm_sec);
   int sps=SND_FD;
   if(F0>=5450) sps=SND_FD2;
   rec->setFormat(sps,16,1);
   rec->open(recfile,"w");
   RECORDING=true;
 }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Upload1Click(TObject *Sender)
{
 if(Upload1->Checked) Upload1->Checked=false;
 else Upload1->Checked=true;
 TIniFile *ini;
 ini = new TIniFile(ChangeFileExt( Application->ExeName, ".INI" ));
 ini->WriteBool("UPLOAD","ENABLE",Upload1->Checked);
 delete ini;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Simple1Click(TObject *Sender)
{
 Simple1->Checked=true;
 SHOW_HOUR=false;
 TIniFile *ini;
 ini = new TIniFile(ChangeFileExt( Application->ExeName, ".INI" ));
 ini->WriteBool("WATERFALL","STACKING",SHOW_HOUR);
 delete ini;
 int x1=paintSpectrum(SPECTRUM_W);
 int x2=paintFreqScale(x1);
 paintSpectrogram(x2);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::withStacking1Click(TObject *Sender)
{
 withStacking1->Checked=true;
 SHOW_HOUR=true;
 TIniFile *ini;
 ini = new TIniFile(ChangeFileExt( Application->ExeName, ".INI" ));
 ini->WriteBool("WATERFALL","STACKING",SHOW_HOUR);
 delete ini;
 int x1=paintSpectrum(SPECTRUM_W);
 int x2=paintFreqScale(x1);
 paintSpectrogram(x2);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Exit1Click(TObject *Sender)
{
    exit(0);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Image3MouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
   if(X<X_SCALE)
   {
     X1=X; Y1=Y;
     Image3->Canvas->Pen->Color = clRed;
     Image3->Canvas->Pen->Style = psSolid;
     Image3->Canvas->Rectangle(X1-2,Y1-2,X1+2,Y1+2);
     double Fy=Fdial+Flow+Fspan*(1.0-1.0*Y/Image3->Height);
     Image3->Canvas->Font->Color=clRed;
      Image3->Canvas->Font->Style = TFontStyles()<< fsBold;
      AnsiString s0;
      s0.sprintf("%.3f Hz",Fy);
      int tw=Image1->Canvas->TextWidth(s0);
      int th=Image1->Canvas->TextHeight(s0);
      if((X1+tw)>=X_SCALE)Image3->Canvas->TextOut(X-tw-2,Y-th-2,s0);
      else Image3->Canvas->TextOut(X-2,Y-th-2,s0);
      Image3->Canvas->Font->Style = TFontStyles();
   }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Image3MouseMove(TObject *Sender, TShiftState Shift,
      int X, int Y)
{

  if(X1>0)
   {
     X2=X; Y2=Y;
     Image3->Canvas->FillRect(Rect(0,0,Image3->Width,Image3->Height));
     Image3->Canvas->Pen->Color = clRed;
     Image3->Canvas->Pen->Style = psSolid;
     Image3->Canvas->Rectangle(X1-2,Y1-2,X1+2,Y1+2);

     Image3->Canvas->Pen->Color = clRed;
     //Image3->Canvas->Pen->Style = psSolid;
     Image3->Canvas->Pen->Style = psDot	;
     Image3->Canvas->MoveTo(X1,Y1);
     Image3->Canvas->LineTo(X2,Y2);

      double Fy=Fdial+Flow+Fspan*(1.0-1.0*Y/Image3->Height);
      Image3->Canvas->Font->Color=clRed;
      Image3->Canvas->Font->Style = TFontStyles()<< fsBold;
      AnsiString s0;
      s0.sprintf("%.3f Hz",Fy);
      int tw=Image1->Canvas->TextWidth(s0);
      int th=Image1->Canvas->TextHeight(s0);
      if((X1+tw)>=X_SCALE)Image3->Canvas->TextOut(X1-tw-2,Y1-th-2,s0);
      else Image3->Canvas->TextOut(X1-2,Y1-th-2,s0);
      Image3->Canvas->Font->Style = TFontStyles();


     double dF=Fspan*(Y1-Y2)/Image3->Height;
     //if(X>=X_SCALE)
     {
      Image3->Canvas->Font->Color=clRed;
      Image3->Canvas->Font->Style = TFontStyles()<< fsBold;
      AnsiString s0;
      s0.sprintf("dF=%.3f Hz",dF);
      int tw=Image1->Canvas->TextWidth(s0);
      int th=Image1->Canvas->TextHeight(s0);
      if((X2+tw)>=X_SCALE)Image3->Canvas->TextOut(X2-tw,Y2-th,s0);
      else Image3->Canvas->TextOut(X2,Y2-th,s0);
      if(X>=X_SCALE)
      {if(Shift.Contains(ssCtrl) || Shift.Contains(ssRight))
            s0.sprintf("Calibration");
       else s0.sprintf("For calibration hold CTRL");
       int tw=Image1->Canvas->TextWidth(s0);
       int th=Image1->Canvas->TextHeight(s0);
       Image3->Canvas->TextOut(X1-tw,Y1+th,s0);
      }
      Image3->Canvas->Font->Style = TFontStyles();
     }
   }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Image3MouseUp(TObject *Sender, TMouseButton Button,
      TShiftState Shift, int X, int Y)
{

 if(X>=X_SCALE && (Shift.Contains(ssCtrl) || Shift.Contains(ssRight)))
 {
   X2=X; Y2=Y;
  //вычислить новый частотный сдвиг
  double DFnew=Fspan*(Y1-Y2)/Image3->Height;
  if(USE_LSB) DFnew=-DFnew;
  CalibratedDF=CalibratedDF - DFnew;

 }
 X1=X2=Y1=Y2=0;
 Image3->Canvas->FillRect(Rect(0,0,Image3->Width,Image3->Height));
}
//---------------------------------------------------------------------------


void __fastcall TForm1::SpeedButton4Click(TObject *Sender)
{
  mycall=Edit2->Text.UpperCase();
  myqth=Edit3->Text;
  TIniFile *ini;
  ini = new TIniFile(ChangeFileExt( Application->ExeName, ".INI" ));
   ini->WriteString("GRABBER","MYCALL",mycall);
   ini->WriteString("GRABBER","MYQTH",myqth);
  delete ini;
 int x1=paintSpectrum(SPECTRUM_W);
 int x2=paintFreqScale(x1);
 paintSpectrogram(x2);
  Label2->Visible=false;
  Edit2->Visible=false;
  Label4->Visible=false;
  Edit3->Visible=false;
  SpeedButton4->Visible=false;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Edit2Enter(TObject *Sender)
{
 Edit2->Text=Edit2->Text.UpperCase();        
}
//---------------------------------------------------------------------------

void __fastcall TForm1::CALLandQTH1Click(TObject *Sender)
{
  Label2->Caption="CALL:"; Label2->Visible=true;
  Edit2->Text=mycall; Edit2->Visible=true;
  Label4->Caption="QTH:"; Label4->Visible=true;
  Edit3->Text=myqth; Edit3->Visible=true;
  SpeedButton4->Visible=true;
}
//---------------------------------------------------------------------------


void __fastcall TForm1::HowsetupCALLandQTH1Click(TObject *Sender)
{
 AnsiString s;

 s="Чтобы ввести свой позывной и локатор:\n";
 s=s+"- войдите в меню File->CALL and QTH;\n";
 s=s+"- в нижней строке откроются поля ввода;\n";
 s=s+"- введите данные и нажмите ОК\n";

 MessageBox(NULL,s.c_str(),"SGR Help",MB_OK|MB_ICONINFORMATION);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::CaptureUploadhelp1Click(TObject *Sender)
{
 AnsiString s;

 s="Сохранение скриншотов и выгрузка на сайт:\n\n";
 s=s+"в меню Capture выберите период сохранения скриншотов и графический формат;\n";
 s=s+"сохраняемый скриншот будет иметь имя capt.bmp (или jpg).\n\n";
 s=s+"Если стоит галочка Capture->Upload, каждый новый скриншот будет выгружаться на сайт,\n";
 s=s+"для чего запускается скрипт upload.bat (используется утилита curl.exe).\n\n";
 s=s+"В файле upload.bat нужно указать адрес Вашего сайта и пароль для доступа по FTP.";

 MessageBox(NULL,s.c_str(),"SGR Help",MB_OK|MB_ICONINFORMATION);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Calibrationhelp1Click(TObject *Sender)
{
 AnsiString s;

 s="Калибровка: меню Calibration\n\n";
 s=s+"- auto (DCF/HGA) - автоматическая калибровка по сигналам\n";
 s=s+"  служебных станций DCF39 или HGA22.\n";
 s=s+"  По достижении необходимой точности найденное значение частотного сдвига\n";
 s=s+"  запоминается и автоматическая калибровка завершается.\n\n";
 s=s+"- costant DF - здесь отображается найденный/введенный частотный сдвиг.\n";
 s=s+"  По окончании автоматической калибровки этот режим активируется.\n\n";
 s=s+"- enter DF - ручной ввод частотного сдвига,\n";
 s=s+"  в нижней строке открывается поле ввода. Введите и нажмите ОК.\n\n";
 s=s+"Уточнить калибровку можно непосредственно на спектрограмме:\n";
 s=s+"наведите курсор на сигнал известной частоты, нажмите и удерживайте\n";
 s=s+"левую кнопку мыши, переместите курсор на шкалу частот, выбрав истинную частоту.\n";
 s=s+"Удерживайте нажатой кнопку CTRL и отпустите кнопку мыши.\n";
 s=s+"Начнется отображение с учетом нового частотного сдвига.\n";
 s=s+"Без CTRL новое значение не будет применено.";
 MessageBox(NULL,s.c_str(),"SGR Help",MB_OK|MB_ICONINFORMATION);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::NoiseBlankerhelp1Click(TObject *Sender)
{
  AnsiString s;

 s="Подавитель импульсных помех NB:\n";
 s=s+"включите его и выберите порог срабатывания.\n";

 MessageBox(NULL,s.c_str(),"SGR Help",MB_OK|MB_ICONINFORMATION);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Frequencyhelp1Click(TObject *Sender)
{
 AnsiString s;

 s="Выбор частот приема:\n\n";
 s=s+"- TA - трансатлантическое QRSS окно, центр 137777 Гц.\n";
 s=s+"  (параметр F_TA в ini-файле).\n\n";
 s=s+"- EU - европейское QRSS окно, центр 136172 Гц.\n";
 s=s+"  (параметр F_EU в ini-файле).\n\n";
 s=s+"- HGA22 - несущая станции HGA22, 135430 Гц.\n";
 s=s+"- DCF39 - несущая станции DCF39, 138830.4 Гц.\n";
 s=s+"  (по этим сигналам удобно проверять калибровку).\n\n";
 s=s+"- Enter Fc - введите произвольную частоту (аудио на выходе ПРМ).\n\n";
 s=s+"- Fdial - выберите (или введите) частоту настройки Вашего приемника\n";
 s=s+"  (если частота настройки выше эфирной частоты сигнала,\n";
 s=s+"   программа переключается в режим LSB - нижней боковой полосы)." ;
 MessageBox(NULL,s.c_str(),"SGR Help",MB_OK|MB_ICONINFORMATION);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::AboutWaterfallStacking1Click(TObject *Sender)
{
 AnsiString s;

 s="Waterfall stacking:\n\n";
 s=s+"режим накопления копий сигнала с периодом 1 час.\n";
 s=s+"При активации режима на спектрограмме красной линией\n";
 s=s+"отмечается граница области накопления.\n";

 MessageBox(NULL,s.c_str(),"SGR Help",MB_OK|MB_ICONINFORMATION);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::AboutSGRA1Click(TObject *Sender)
{
 AnsiString s;

 s="Spectrum Grabber and Analizer SGR A*:\n";
 s=s+"программа спектроанализатора с минимумом настроек,\n";
 s=s+"служащая для создания автоматических грабберов диапазона 136 кГц.\n";
 s=s+"Имеется режим автоматической калибровки по сигналам служебных станций.\n\n";
 s=s+"RN3AUS/Alex January 2022.";
 MessageBox(NULL,s.c_str(),"SGR Help",MB_OK|MB_ICONINFORMATION);
}
//---------------------------------------------------------------------------

