#ifndef IS_ENHANCED
	#error Необходимо расширенное издание Inno Setup (restools) для компилляции этого скрипта
#endif

#define PathOut      "'C:\Program Files\*'"				/* откуда копируем */
#define PathIn     	 "'C:\123'"         					/* куда копируем	 */

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
var
 Button1: TButton;
 CopyInfoLabel, FileInfoLabel: TLabel;
 AllSize: Integer;

// PathOut - Откуда копировать. Можно указывать маски для копирования, или оставить звездочку и будет копироваться все
// Для копирования отдельных файлов прописываем их в PathOut, и обязательно ставим bInnerFolders как false
// PathIn - Куда копировать
// bInnerFolders - Включать ли подпапки
function isCopyFile(callback: Longword; PathOut, PathIn: String; bInnerFolders: bool): integer; external 'isCopyFile@files:ISCopyFile.dll stdcall';
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

function mCallback( what: PAnsiChar; int1: Longint; str: PAnsiChar): Boolean;
begin
	if (string(what) = 'allsize') then
		AllSize:= int1;

	if(string(what) = 'filename') then
		FileInfoLabel.Caption:= MinimizePathName(str, WizardForm.StatusLabel.Font, WizardForm.StatusLabel.Width);

	if (string(what) = 'write') then begin
		CopyInfoLabel.Caption:= 'Скопировано '+MbOrTb(int1) + ' из ' + MbOrTb(AllSize);
		WizardForm.ProgressGauge.Position:= Round(int1);
		WizardForm.ProgressGauge.Max:= AllSize;
	end;
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

  FileInfoLabel:= TLabel.Create(WizardForm);
  with FileInfoLabel do begin
    Left:= WizardForm.ProgressGauge.Left;
    Top:= WizardForm.ProgressGauge.Top+WizardForm.ProgressGauge.Height+ScaleY(25);
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

			mCopyFiles({#PathOut}, {#PathIn});
		end;
  end;
end;
