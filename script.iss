[Setup]
AppName=My Program
AppVerName=My Program version 1.5
DefaultDirName={pf}\My Program
DefaultGroupName=My Program

[Languages]
Name: rus; MessagesFile: compiler:Languages\Russian.isl

[Files]
Source: ISCopyFile.dll; Flags: dontcopy

[Icons]
Name: {group}\Uninstall; IconFilename: {app}\unins000.exe; Filename: {app}\unins000.exe

[Code]
const
  PM_REMOVE = 1;
	
var
 Button1: TButton;
 CopyInfoLabel: TLabel;
 str: AnsiString;

// PathOut - Откуда копировать. Можно указывать маски для копирования, или оставить звездочку и будет копироваться все 
// Для копирования отдельных файлов прописываем их в PathOut, и обязательно ставим bInnerFolders как false
// PathIn - Куда копировать
// bInnerFolders - Включать ли подпапки
function isCopyFile(callback: Longword; PathOut,PathIn: String; bInnerFolders: bool): integer; external 'isCopyFile@files:ISCopyFile.dll stdcall';
procedure BreakCopy(); external 'BreakCopy@files:ISCopyFile.dll stdcall';
 
procedure Button1OnClick(Sender: TObject);
begin
  BreakCopy();
end;

function MbOrTb(Float: Extended): String;
begin
	if Float/1024 < 1024 then Result:= format('%.2n', [Float/1024])+' МБ' else
		Result:= format('%.2n', [Float/(1024*1024)])+' ГБ'; 
    StringChange(Result, ',', '.');
end;

// AllSize - Общий размер для копирования
// mCopySize - Сколько всего скопировано
// str - Копирующийся файл
function mCallback(AllSize,mCopySize: Longint; str: AnsiString): Boolean;
begin
	CopyInfoLabel.Caption:= 'Скопировано '+MbOrTb(mCopySize)+' из '+MbOrTb(AllSize);
	WizardForm.ProgressGauge.Position:= Round(mCopySize);
	WizardForm.ProgressGauge.Max:= AllSize;
	Application.ProcessMessages;
end;

procedure mCopyFiles(mOut: String; mIn: String);
var
	callback: Longword;
begin
  callback:= CallbackAddr('mCallback');
  try
   isCopyFile(callback, mOut, mIn, True);
   Button1.visible:= false;
  except
   MsgBox('Failed!', mbError, MB_OK);
   Button1.visible:= false;
  end;
end;

procedure InitializeWizard();
begin
	CopyInfoLabel:= TLabel.Create(WizardForm);
  with CopyInfoLabel do begin
    Left:= WizardForm.ProgressGauge.Left;
    Top:= WizardForm.ProgressGauge.Top+WizardForm.ProgressGauge.Height+ScaleY(10);
    Width:= WizardForm.StatusLabel.Width;
    Height:= WizardForm.StatusLabel.Height;
    AutoSize:= False;
    Transparent:= True;
    Parent:= WizardForm.InstallingPage;
  end;
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
	case CurStep of
		ssPostInstall: begin
			WizardForm.ProgressGauge.Position:= 0;
			Button1:= TButton.create(WizardForm);
			with Button1 do begin
				Parent:= WizardForm;
				Width:= 135;
				Height:= WizardForm.Cancelbutton.Height;
				Caption:= 'Отмена копирования';
				Left:= 352;
				Top:= WizardForm.Cancelbutton.top;
				OnClick:= @Button1OnClick;
			end;
			
			mCopyFiles('C:\123\*', 'C:\321');
		end;
  end;
end;
