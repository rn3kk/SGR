//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
USEFORM("sgr1.cpp", Form1);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
        try
        {
                 Application->Initialize();
                 Application->Title = "Spectrum Grabber and Analizer (SGR A*)";
                 Application->CreateForm(__classid(TForm1), &Form1);
                 Application->Run();
        }
        catch (Exception &exception)
        {
                 Application->ShowException(&exception);
        }
        return 0;
}
//---------------------------------------------------------------------------
