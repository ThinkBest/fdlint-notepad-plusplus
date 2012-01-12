program NPPPlugin;

uses
  Forms,
  SysUtils,
  Windows,
  Registry,
  TLHelp32,
  ShlObj,
  ShellAPI;

const
  FileList : array[1..3] of String = (
    'NppFDevXRay.dll',
    'XRay\xray.exe',
    'XRay\NPPPluginUpdater.exe'
  );

var
  I:Integer;
  Path:String;
  NPPPath:String;

function SelectFolderDialog(const Handle:integer;const Caption:string;
              const InitFolder:string;var SelectedFolder:string):boolean;
var
  BInfo: _browseinfoA;
  Buffer: array[0..MAX_PATH] of Char;
  ID: IShellFolder;
  Eaten, Attribute: Cardinal;
  ItemID: PItemidlist;
begin
  with BInfo do
  begin
    HwndOwner := Handle;
    lpfn := nil;
    lpszTitle := Pchar(Caption);
    ulFlags := BIF_RETURNONLYFSDIRS;
    SHGetDesktopFolder(ID);
    ID.ParseDisplayName(0,nil,'\',Eaten,ItemID,Attribute);
    pidlRoot := ItemID;
    GetMem(pszDisplayName, MAX_PATH);
  end;
  if SHGetPathFromIDList(SHBrowseForFolder(BInfo), Buffer) then
  begin
    SelectedFolder := Buffer;
    if Length(SelectedFolder)<>3 then SelectedFolder := SelectedFolder + '\';
    result := True;
  end
  else begin
    SelectedFolder := '\';
    result := False;
  end;
end;

function GetNPPDir():String;
begin
  Result:='';
  with TRegistry.Create() do
  begin
    RootKey:=HKEY_LOCAL_MACHINE;
    OpenKey('SOFTWARE',True);
    OpenKey('Notepad++',True);
    Result:=ReadString('');
    CloseKey;
  end;
  if((DirectoryExists(Result+'\plugins'))and(FileExists(Result+'\notepad++.exe')))then
  begin
    Result:=Result+'\';
    exit;
  end
  else
  begin
    if(SelectFolderDialog(Application.Handle,'���ֹ�ѡ��Notepad++��װĿ¼','C:\',Result))then
    begin
       if((DirectoryExists(Result+'plugins'))and(FileExists(Result+'notepad++.exe')))then
        exit
       else
       begin
        Application.MessageBox('��ѡ����ȷ��Notepad++��װĿ¼','��ʾ');
        Result:='';
       end;
    end
    else
    begin
      Result:='';
    end;
  end;
end;  

function FindProcess(AFileName: string): boolean;
var
  hSnapshot: THandle;//���ڻ�ý����б�
  lppe: TProcessEntry32;//���ڲ��ҽ���
  Found: Boolean;//�����жϽ��̱����Ƿ����
begin
  Result :=False;
  hSnapshot := CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);//���ϵͳ�����б�
  lppe.dwSize := SizeOf(TProcessEntry32);//�ڵ���Process32First API֮ǰ����Ҫ��ʼ��lppe��¼�Ĵ�С
  Found := Process32First(hSnapshot, lppe);//�������б�ĵ�һ��������Ϣ����ppe��¼��
  while Found do
  begin
    if ((UpperCase(ExtractFileName(lppe.szExeFile))=UpperCase(AFileName)) or (UpperCase(lppe.szExeFile )=UpperCase(AFileName))) then
    begin
      Result :=True;
      exit;
    end;
    Found := Process32Next(hSnapshot, lppe);//�������б����һ��������Ϣ����lppe��¼��
  end;
end;

begin
  Path:=ExtractFilePath(Application.ExeName);
  for I:=1 to Length(FileList) do
  begin
    if(not FileExists(Path+FileList[I]))then
    begin
     Application.MessageBox('ȱ�ٰ�װ�ļ�','��ʾ');
     Exit;
    end;
  end;
  if(FindProcess('notepad++.exe'))then
  begin
    Application.MessageBox('Notepad++��������','��ʾ');
    exit;
  end;
  NPPPath:=GetNPPDir;
  if(NPPPath='')then
  begin
    exit;
  end;
  CreateDirectory(PChar(NPPPath+'plugins\XRay'),nil);
  for I:=1 to Length(FileList) do
  begin
    CopyFile(PChar(Path+FileList[I]),PChar(NPPPath+'plugins\'+FileList[I]),false);
  end;
  Application.MessageBox('FDLint for Notepad++��װ�ɹ�','��ʾ');
end.
