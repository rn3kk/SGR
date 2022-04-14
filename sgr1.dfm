object Form1: TForm1
  Left = 238
  Top = 155
  BorderIcons = [biSystemMenu, biMinimize]
  BorderStyle = bsSingle
  Caption = 'Spectrum Grabber SGR A*  (ver 1.0)'
  ClientHeight = 453
  ClientWidth = 800
  Color = clBtnFace
  Font.Charset = RUSSIAN_CHARSET
  Font.Color = clWindowText
  Font.Height = -15
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  Menu = MainMenu1
  OldCreateOrder = False
  Position = poDefault
  OnClose = FormClose
  PixelsPerInch = 96
  TextHeight = 16
  object Image1: TImage
    Left = 0
    Top = 0
    Width = 800
    Height = 412
  end
  object Image2: TImage
    Left = 720
    Top = 413
    Width = 80
    Height = 22
    Hint = 'Click to setup Brightness'
    ParentShowHint = False
    ShowHint = True
    OnMouseDown = Image2MouseDown
  end
  object Label1: TLabel
    Left = 552
    Top = 416
    Width = 85
    Height = 16
    BiDiMode = bdLeftToRight
    Caption = 'Cntrst/Brghtns:'
    Font.Charset = RUSSIAN_CHARSET
    Font.Color = clWindowText
    Font.Height = -15
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentBiDiMode = False
    ParentFont = False
  end
  object SpeedButton1: TSpeedButton
    Left = 65
    Top = 413
    Width = 57
    Height = 22
    Hint = 'Audio unput record start/stop'
    Caption = 'Rec'
    Flat = True
    Font.Charset = RUSSIAN_CHARSET
    Font.Color = clWindowText
    Font.Height = -15
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    NumGlyphs = 2
    ParentFont = False
    ParentShowHint = False
    ShowHint = True
    OnClick = SpeedButton1Click
  end
  object SpeedButton2: TSpeedButton
    Left = 0
    Top = 413
    Width = 65
    Height = 22
    Hint = 'Make screenshot now'
    Caption = 'Capture'
    Flat = True
    Font.Charset = RUSSIAN_CHARSET
    Font.Color = clWindowText
    Font.Height = -15
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
    ParentShowHint = False
    ShowHint = True
    OnClick = SpeedButton2Click
  end
  object Label2: TLabel
    Left = 128
    Top = 416
    Width = 48
    Height = 16
    Caption = 'Label2'
    Font.Charset = RUSSIAN_CHARSET
    Font.Color = clRed
    Font.Height = -15
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
    Visible = False
  end
  object Label3: TLabel
    Left = 376
    Top = 416
    Width = 64
    Height = 16
    Caption = 'Offset [Hz]:'
    Visible = False
  end
  object SpeedButton3: TSpeedButton
    Left = 516
    Top = 413
    Width = 25
    Height = 22
    Caption = 'OK'
    Flat = True
    Font.Charset = RUSSIAN_CHARSET
    Font.Color = clWindowText
    Font.Height = -15
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
    Visible = False
    OnClick = SpeedButton3Click
  end
  object Image3: TImage
    Left = 0
    Top = 0
    Width = 800
    Height = 412
    Cursor = crCross
    Transparent = True
    OnMouseDown = Image3MouseDown
    OnMouseMove = Image3MouseMove
    OnMouseUp = Image3MouseUp
  end
  object Label4: TLabel
    Left = 248
    Top = 416
    Width = 37
    Height = 16
    Caption = 'QTH:'
    Font.Charset = RUSSIAN_CHARSET
    Font.Color = clRed
    Font.Height = -15
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
    Visible = False
  end
  object SpeedButton4: TSpeedButton
    Left = 352
    Top = 416
    Width = 23
    Height = 22
    Caption = 'OK'
    Flat = True
    Font.Charset = RUSSIAN_CHARSET
    Font.Color = clWindowText
    Font.Height = -15
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
    Visible = False
    OnClick = SpeedButton4Click
  end
  object StatusBar1: TStatusBar
    Left = 0
    Top = 434
    Width = 800
    Height = 19
    Panels = <
      item
        Text = 'Fdial=136000'
        Width = 80
      end
      item
        Text = 'Fc=1777'
        Width = 60
      end
      item
        Text = 'Volume=-10 dB'
        Width = 90
      end
      item
        Text = 'default audio'
        Width = 130
      end
      item
        Text = 'NB'
        Width = 50
      end
      item
        Text = 'DF'
        Width = 130
      end
      item
        Width = 50
      end>
    SimplePanel = False
  end
  object TrackBar1: TTrackBar
    Left = 640
    Top = 413
    Width = 80
    Height = 22
    Hint = 'Contrast'
    Max = 300
    Min = 30
    Orientation = trHorizontal
    ParentShowHint = False
    Frequency = 1
    Position = 30
    SelEnd = 0
    SelStart = 0
    ShowHint = True
    TabOrder = 1
    ThumbLength = 19
    TickMarks = tmBottomRight
    TickStyle = tsAuto
    OnChange = TrackBar1Change
  end
  object Edit1: TEdit
    Left = 448
    Top = 416
    Width = 64
    Height = 20
    Hint = 'Calibration frequency offset, Hz'
    BorderStyle = bsNone
    Font.Charset = RUSSIAN_CHARSET
    Font.Color = clWindowText
    Font.Height = -15
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
    TabOrder = 2
    Text = 'Edit1'
    Visible = False
    OnChange = Edit1Change
    OnKeyPress = Edit1KeyPress
  end
  object Edit2: TEdit
    Left = 176
    Top = 416
    Width = 65
    Height = 20
    BorderStyle = bsNone
    Font.Charset = RUSSIAN_CHARSET
    Font.Color = clWindowText
    Font.Height = -15
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
    TabOrder = 3
    Visible = False
    OnEnter = Edit2Enter
  end
  object Edit3: TEdit
    Left = 288
    Top = 416
    Width = 57
    Height = 20
    BorderStyle = bsNone
    Font.Charset = RUSSIAN_CHARSET
    Font.Color = clWindowText
    Font.Height = -15
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
    TabOrder = 4
    Visible = False
  end
  object MainMenu1: TMainMenu
    Left = 144
    Top = 24
    object File1: TMenuItem
      Caption = 'File'
      object Openwav1: TMenuItem
        Caption = 'Open wav...'
        OnClick = Openwav1Click
      end
      object N3: TMenuItem
        Caption = '-'
      end
      object CALLandQTH1: TMenuItem
        Caption = 'CALL and QTH'
        OnClick = CALLandQTH1Click
      end
      object N12: TMenuItem
        Caption = '-'
      end
      object Exit1: TMenuItem
        Caption = 'Exit'
        OnClick = Exit1Click
      end
    end
    object Device1: TMenuItem
      Caption = 'Device'
      object default1: TMenuItem
        Caption = 'default'
        Checked = True
        RadioItem = True
        OnClick = default1Click
      end
      object N1: TMenuItem
        Caption = '-'
      end
      object device01: TMenuItem
        Caption = 'device0'
        RadioItem = True
        Visible = False
        OnClick = device01Click
      end
      object device11: TMenuItem
        Caption = 'device1'
        RadioItem = True
        Visible = False
        OnClick = device11Click
      end
      object device21: TMenuItem
        Caption = 'device2'
        RadioItem = True
        Visible = False
        OnClick = device21Click
      end
      object device31: TMenuItem
        Caption = 'device3'
        RadioItem = True
        Visible = False
        OnClick = device31Click
      end
      object device41: TMenuItem
        Caption = 'device4'
        Visible = False
        OnClick = device41Click
      end
      object device51: TMenuItem
        Caption = 'device5'
        Visible = False
        OnClick = device51Click
      end
      object device61: TMenuItem
        Caption = 'device6'
        Visible = False
        OnClick = device61Click
      end
      object device71: TMenuItem
        Caption = 'device7'
        Visible = False
        OnClick = device71Click
      end
      object device81: TMenuItem
        Caption = 'device8'
        Visible = False
        OnClick = device81Click
      end
    end
    object Capture1: TMenuItem
      Caption = 'Capture'
      object disabled1: TMenuItem
        Caption = 'disabled'
        Checked = True
        RadioItem = True
        OnClick = disabled1Click
      end
      object N1min1: TMenuItem
        Caption = '1 min'
        RadioItem = True
        OnClick = N1min1Click
      end
      object N3min1: TMenuItem
        Caption = '3 min'
        RadioItem = True
        OnClick = N3min1Click
      end
      object N5min1: TMenuItem
        Caption = '5 min'
        RadioItem = True
        OnClick = N5min1Click
      end
      object N10min1: TMenuItem
        Caption = '10 min'
        RadioItem = True
        OnClick = N10min1Click
      end
      object N15min1: TMenuItem
        Caption = '15 min'
        RadioItem = True
        OnClick = N15min1Click
      end
      object N20min1: TMenuItem
        Caption = '20 min'
        RadioItem = True
        OnClick = N20min1Click
      end
      object N30min1: TMenuItem
        Caption = '30 min'
        RadioItem = True
        OnClick = N30min1Click
      end
      object N1hour1: TMenuItem
        Caption = '60 min'
        RadioItem = True
        OnClick = N1hour1Click
      end
      object userperiod1: TMenuItem
        Caption = 'user period'
        RadioItem = True
        Visible = False
        OnClick = userperiod1Click
      end
      object N5: TMenuItem
        Caption = '-'
      end
      object BMP1: TMenuItem
        Caption = '*.bmp'
        Checked = True
        GroupIndex = 1
        RadioItem = True
        OnClick = BMP1Click
      end
      object JPG1: TMenuItem
        Caption = '*.jpg'
        GroupIndex = 1
        RadioItem = True
        OnClick = JPG1Click
      end
      object N11: TMenuItem
        Caption = '-'
        GroupIndex = 1
      end
      object Upload1: TMenuItem
        Caption = 'Upload'
        GroupIndex = 1
        OnClick = Upload1Click
      end
    end
    object Calibration1: TMenuItem
      Caption = 'Calibration'
      object autoDCFHGA1: TMenuItem
        Caption = 'auto (DCF/HGA)'
        Checked = True
        RadioItem = True
        OnClick = autoDCFHGA1Click
      end
      object fixedvalue1: TMenuItem
        Caption = 'constant dF'
        RadioItem = True
        OnClick = fixedvalue1Click
      end
      object N2: TMenuItem
        Caption = '-'
      end
      object enterdF1: TMenuItem
        Caption = 'enter dF...'
        OnClick = enterdF1Click
      end
    end
    object NB1: TMenuItem
      Caption = 'NB'
      object enable1: TMenuItem
        Caption = 'Enable'
        Checked = True
        OnClick = enable1Click
      end
      object N4: TMenuItem
        Caption = '-'
      end
      object Level1: TMenuItem
        Caption = 'Level'
        object N3dB1: TMenuItem
          Caption = '3 dB'
          OnClick = N3dB1Click
        end
        object N6dB1: TMenuItem
          Caption = '6 dB'
          RadioItem = True
          OnClick = N6dB1Click
        end
        object N10dB1: TMenuItem
          Caption = '10 dB'
          RadioItem = True
          OnClick = N10dB1Click
        end
        object N15dB1: TMenuItem
          Caption = '15 dB'
          RadioItem = True
          OnClick = N15dB1Click
        end
        object N20dB1: TMenuItem
          Caption = '20 dB'
          RadioItem = True
          OnClick = N20dB1Click
        end
        object N30dB1: TMenuItem
          Caption = '30 dB'
          RadioItem = True
          OnClick = N30dB1Click
        end
        object user1: TMenuItem
          Caption = 'user'
          RadioItem = True
          Visible = False
          OnClick = user1Click
        end
      end
    end
    object Freq1: TMenuItem
      Caption = 'Freq'
      object TA1: TMenuItem
        Caption = 'TA'
        Checked = True
        RadioItem = True
        OnClick = TA1Click
      end
      object EU1: TMenuItem
        Caption = 'EU'
        RadioItem = True
        OnClick = EU1Click
      end
      object user3: TMenuItem
        Caption = 'user'
        RadioItem = True
        Visible = False
        OnClick = user3Click
      end
      object N8: TMenuItem
        Caption = '-'
      end
      object HGA221: TMenuItem
        Caption = 'HGA-22'
        RadioItem = True
        OnClick = HGA221Click
      end
      object DCF391: TMenuItem
        Caption = 'DCF-39'
        RadioItem = True
        OnClick = DCF391Click
      end
      object N9: TMenuItem
        Caption = '-'
      end
      object EnterFc1: TMenuItem
        Caption = 'Enter Fc...'
        OnClick = EnterFc1Click
      end
      object N7: TMenuItem
        Caption = '-'
      end
      object Fdial1: TMenuItem
        Caption = 'Fdial'
        object N1350002: TMenuItem
          Caption = '135000'
          RadioItem = True
          OnClick = N1350002Click
        end
        object N1355002: TMenuItem
          Caption = '135500'
          RadioItem = True
          OnClick = N1355002Click
        end
        object N1360001: TMenuItem
          Caption = '136000'
          Checked = True
          RadioItem = True
          OnClick = N1360001Click
        end
        object N1370001: TMenuItem
          Caption = '137000'
          RadioItem = True
          OnClick = N1370001Click
        end
        object N1250001: TMenuItem
          Caption = '125000'
          RadioItem = True
          OnClick = N1250001Click
        end
        object user2: TMenuItem
          Caption = 'user'
          RadioItem = True
          Visible = False
        end
        object N6: TMenuItem
          Caption = '-'
        end
        object EnterFdial1: TMenuItem
          Caption = 'Enter Fdial...'
          OnClick = EnterFdial1Click
        end
      end
    end
    object Waterfall1: TMenuItem
      Caption = 'Waterfall'
      object Simple1: TMenuItem
        Caption = 'Simple'
        Checked = True
        RadioItem = True
        OnClick = Simple1Click
      end
      object withStacking1: TMenuItem
        Caption = 'with Stacking'
        RadioItem = True
        OnClick = withStacking1Click
      end
    end
    object Colors1: TMenuItem
      Caption = 'Colors'
      object Spectrum1: TMenuItem
        Caption = 'Spectrum...'
        OnClick = Spectrum1Click
      end
      object SlowSpectrum1: TMenuItem
        Caption = 'Slow Spectrum...'
        OnClick = SlowSpectrum1Click
      end
      object Backgfound1: TMenuItem
        Caption = 'Background...'
        OnClick = Backgfound1Click
      end
      object N10: TMenuItem
        Caption = '-'
      end
      object default2: TMenuItem
        Caption = 'default'
        OnClick = default2Click
      end
    end
    object Help1: TMenuItem
      Caption = 'Help'
      object AboutSGRA1: TMenuItem
        Caption = 'About SGR A* '
        OnClick = AboutSGRA1Click
      end
      object N13: TMenuItem
        Caption = '-'
      end
      object HowsetupCALLandQTH1: TMenuItem
        Caption = 'How setup CALL and QTH?'
        OnClick = HowsetupCALLandQTH1Click
      end
      object CaptureUploadhelp1: TMenuItem
        Caption = 'Capture/Upload help'
        OnClick = CaptureUploadhelp1Click
      end
      object Calibrationhelp1: TMenuItem
        Caption = 'Calibration help'
        OnClick = Calibrationhelp1Click
      end
      object NoiseBlankerhelp1: TMenuItem
        Caption = 'Noise Blanker help'
        OnClick = NoiseBlankerhelp1Click
      end
      object Frequencyhelp1: TMenuItem
        Caption = 'Frequency help'
        OnClick = Frequencyhelp1Click
      end
      object AboutWaterfallStacking1: TMenuItem
        Caption = 'About Waterfall Stacking'
        OnClick = AboutWaterfallStacking1Click
      end
    end
  end
  object Timer1: TTimer
    Interval = 2000
    OnTimer = Timer1Timer
    Left = 128
    Top = 80
  end
  object Timer2: TTimer
    Interval = 10000
    OnTimer = Timer2Timer
    Left = 200
    Top = 80
  end
  object OpenDialog1: TOpenDialog
    Left = 48
    Top = 16
  end
  object ColorDialog1: TColorDialog
    Ctl3D = True
    Left = 384
    Top = 40
  end
end
