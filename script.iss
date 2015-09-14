[Setup]
AppName=My Program
AppVerName=My Program version 1.5
DefaultDirName={pf}\My Program
DefaultGroupName=My Program
Compression=zip

[Languages]
;Name: rus; MessagesFile: compiler:Languages\Russian.isl

[Files]
Source: innocallback.dll; Flags: dontcopy
Source: ISCopyFile.dll; Flags: dontcopy

[Icons]
Name: {group}\Uninstall; IconFilename: {app}\unins000.exe; Filename: {app}\unins000.exe

[Code]

function PeekMessage(var lpMsg: TMsg; hWnd: HWND; wMsgFilterMin, wMsgFilterMax, wRemoveMsg: UINT): BOOL; external 'PeekMessageA@user32.dll stdcall';
function TranslateMessage(const lpMsg: TMsg): BOOL; external 'TranslateMessage@user32.dll stdcall';
function DispatchMessage(const lpMsg: TMsg): Longint; external 'DispatchMessageA@user32.dll stdcall';

const
  PM_REMOVE      = 1;
  oneMB=1024*1024;

procedure AppProcessMessage;
var
  Msg: TMsg;
begin
  while PeekMessage(Msg, WizardForm.Handle, 0, 0, PM_REMOVE) do begin
    TranslateMessage(Msg);
    DispatchMessage(Msg);
  end;
end;

type
// AllSize - Общий размер для копирования, в кб
// mCopySize - Сколько всего скопировано, в кб
// str - Копирующийся файл
 TCallback = function (AllSize, mCopySize: Integer; str: String): Integer; 

function WrapCallback (callback: TCallback; paramcount: integer):longword;
  external 'wrapcallback@files:innocallback.dll stdcall';

// PathOut - Откуда копировать. Можно указывать маски для копирования, или оставить звездочку и будет копироваться все. Для копирования отдельных файлов прописываем их в PathOut, и обязательно ставим bInnerFolders как false
// PathIn - Куда копировать
// bInnerFolders - Включать ли подпапки
function isCopyFile( callback: longword; PathOut, PathIn: String; bInnerFolders: bool ): integer; external 'isCopyFile@files:ISCopyFile.dll stdcall';
procedure BreakCopy( mCopy: bool ); external 'BreakCopy@files:ISCopyFile.dll stdcall';



var
 Button1:     TButton;
 CopyInfoLabel: TLabel;
 
procedure InitializeWizard();
begin
CopyInfoLabel:= TLabel.Create(WizardForm);
  with CopyInfoLabel do
  begin
    Left:= WizardForm.ProgressGauge.Left;
    Top:= WizardForm.ProgressGauge.Top + WizardForm.ProgressGauge.Height + ScaleY(10);
    Width:= WizardForm.StatusLabel.Width;
    Height:= WizardForm.StatusLabel.Height;
    AutoSize:= False;
    Transparent := True;
    Parent:= WizardForm.InstallingPage;
   end;
end;

procedure Button1OnClick(Sender: TObject);
begin
  BreakCopy( false );
end;

Function NumToStr(Float: Extended): String;
Begin
  Result:= Format('%.2n', [Float]); StringChange(Result, ',', '.');
  while ((Result[Length(Result)] = '0') or (Result[Length(Result)] = '.')) and (Pos('.', Result) > 0) do
    SetLength(Result, Length(Result)-1);
End;

function mCallback (AllSize, mCopySize: Integer; str: String): Integer;
begin
AppProcessMessage;
	CopyInfoLabel.Caption:= 'Скопировано ' + NumToStr( mCopySize ) + ' кб из ' + NumToStr( AllSize ) + ' кб';
  AppProcessMessage;
end;

procedure mCopyFiles( mOut: String; mIn: String );
var callback: longword;
    res: Integer;
begin
  AppProcessMessage;
  callback:=WrapCallback(@mCallback,3);
  try
   isCopyFile( callback, mOut, mIn, True );
   Button1.visible:=false;
  except
   MsgBox('Failed!', mbError, MB_OK);
   Button1.visible:=false;
  end;
end;

procedure CurStepChanged(CurStep: TSetupStep);
var app: String;
begin
  If CurStep=ssPostInstall then
   begin
    Button1:=TButton.create(WizardForm);
    Button1.parent:=WizardForm;
    Button1.width:=135;
    Button1.caption:='Cancel copy';
    Button1.left:=260;
    Button1.top:=WizardForm.cancelbutton.top;
    Button1.OnClick:=@Button1OnClick;

    mCopyFiles( 'C:\123\*', 'C:\321' );
   end;
end;
