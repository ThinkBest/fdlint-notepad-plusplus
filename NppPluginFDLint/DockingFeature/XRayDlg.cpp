//this file is part of notepad++
//Copyright (C)2003 Don HO ( donho@altern.org )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "XRayDlg.h"
#include "PluginDefinition.h"

extern NppData nppData;

//�����µ��̣߳����߳���ɨ��
DWORD WINAPI ThreadScan (LPVOID lpParam)
{
	//ǿ�Ʊ����ļ�
	//��ֹ������������µ��ļ���ʧ
	::SendMessage(nppData._nppHandle, NPPM_SAVECURRENTFILE, 0, 0);
	
	//H��ɨ�贰�ڵľ��
	HWND H = (HWND)lpParam;
	//����״̬�ı�
	::SetWindowText(::GetDlgItem(H,IDC_STATUS),TEXT("����ɨ�����Ժ�......"));
	//���ListBox
	::SendMessage(::GetDlgItem(H,IDC_RESULTLIST),LB_RESETCONTENT,0,0);

	TCHAR FilePath[MAX_PATH];
	TCHAR PluginPath[MAX_PATH];
	TCHAR EXEPath[MAX_PATH] =TEXT("");
	TCHAR TempFilePath[MAX_PATH] =TEXT("");
	TCHAR cmdLine[1024] =TEXT("");
	//�õ�NPP���Ŀ¼
	::SendMessage(nppData._nppHandle, NPPM_GETNPPDIRECTORY, 0, (LPARAM)PluginPath);
	lstrcat(PluginPath,TEXT("\\plugins\\XRay\\"));
	//�õ���ǰ�ļ�·��
	::SendMessage(nppData._nppHandle, NPPM_GETFULLCURRENTPATH, 0, (LPARAM)FilePath);
	//��װ��ִ���ļ�·��
	lstrcat(EXEPath,PluginPath);
	lstrcat(EXEPath,TEXT("xray.exe"));
	//��װ��ʱ�ļ�·��
	lstrcat(TempFilePath,PluginPath);
	lstrcat(TempFilePath,TEXT("output.txt"));
	//��װ������
	lstrcat(cmdLine,EXEPath);
	//ָ��xray�����ʽ
	lstrcat(cmdLine,TEXT(" --format=vim \""));
	lstrcat(cmdLine,FilePath);
	lstrcat(cmdLine,TEXT("\""));
	//�����õ�������
	//TCHAR cmdLine[MAX_PATH] = TEXT("ipconfig /all");
	::OutputDebugString(cmdLine);
	//����SA
	SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};

	//����si��pi
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	//������ʱ�ļ�
	HANDLE hFile = ::CreateFile(TempFilePath,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,&sa,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		//�ļ�����ʧ��
		::SetWindowText(::GetDlgItem(H,IDC_STATUS),TEXT("������ʱ�ļ�ʧ��"));	
		return 0;
	}

	//si����ռ䣬����������
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	si.hStdError = hFile;
	si.hStdOutput = hFile;
	si.wShowWindow = SW_HIDE;  

	//��������
	BOOL ret = ::CreateProcess(NULL, cmdLine, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
	if(ret){
		//�ȴ�60����߽��̽���
		if(::WaitForSingleObject(pi.hProcess,600000)==WAIT_OBJECT_0){
				//ȡ����ʱ�ļ��ߴ�
				DWORD dwFileSize=::GetFileSize(hFile, NULL); 
				//����ռ�
				char* Output = new char[dwFileSize+1];
				DWORD bytesRead;
				//�ƶ���дָ�뵽�ļ�ͷ��
				::SetFilePointer(hFile,0,0,FILE_BEGIN);
				//��ȡ�ļ�ȫ������
				::ReadFile(hFile,Output,dwFileSize,&bytesRead,NULL);
				Output[bytesRead]='\0';
				//�ر��ļ�
				CloseHandle(hFile);
				//UTF-8תUnicode
				int wcsLen = ::MultiByteToWideChar(CP_UTF8,NULL,Output,bytesRead,NULL,0);
				TCHAR* T = new TCHAR[wcsLen+1];
				::MultiByteToWideChar(CP_UTF8,NULL,Output,bytesRead,T,wcsLen);
				T[wcsLen+1] = '\0';
				//���зָ�
				TCHAR * lpch = T;
				TCHAR * pch = wcschr(T,'\n');
				int len;
				while(pch!=NULL){
					len = pch - lpch +1;
					TCHAR* Line = new TCHAR[len+1];
					lstrcpyn(Line,lpch,len);
					Line[len] = '\0';
					//����Listbox
					::SendMessage(::GetDlgItem(H,IDC_RESULTLIST),LB_INSERTSTRING,-1,(LPARAM)Line);
					delete[] Line;
					lpch = pch;
					pch = wcschr(pch+1,'\n');
				}
	
				delete[] Output;
				delete[] T;
				//����״̬�ı�
				::SetWindowText(::GetDlgItem(H,IDC_STATUS),TEXT("ɨ�����"));
		}
		else{
			//��ֹ����
			::TerminateProcess(pi.hProcess,0);
			//�ر��ļ�
			CloseHandle(hFile);
			::SetWindowText(::GetDlgItem(H,IDC_STATUS),TEXT("�ȴ���ʱ"));
		}
	}

	return 0;
}

void XRayDlg::Scan(){		
	//���Notepad++ʶ����ļ�����
	int LANGTYPE = ::SendMessage(nppData._nppHandle, NPPM_GETBUFFERLANGTYPE,::SendMessage(nppData._nppHandle, NPPM_GETCURRENTBUFFERID, 0, 0), 0);
	if((LANGTYPE==-1)||((LANGTYPE!=LangType(L_HTML))&&(LANGTYPE!=LangType(L_JS))&&(LANGTYPE!=LangType(L_CSS)))){
		::MessageBox(_hSelf,TEXT("ֻ��ɨ��HTML��JS����CSS�ļ�"),TEXT("�ļ�����"),MB_OK);
		return;
	}
	//TCHAR EXT[MAX_PATH];
	//�õ���չ��
	//SendMessage(nppData._nppHandle, NPPM_GETEXTPART, 0, (LPARAM)EXT);
	//����ɨ���߳�
	DWORD dwThreadId;
	::CreateThread(NULL,0,ThreadScan,_hSelf,0,&dwThreadId);
	::OutputDebugString(TEXT("Create Thread"));
	return;
}

BOOL CALLBACK XRayDlg::run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{
		case WM_COMMAND : 
		{
			switch (LOWORD(wParam))
			{
				case IDC_RESULTLIST:
				{
					//˫��ListBox����
					if(HIWORD(wParam)==LBN_DBLCLK){
						int index;
						int len;
						//ȡ��ѡ����Ŀ��index
						index = ::SendDlgItemMessage(_hSelf,IDC_RESULTLIST,LB_GETCARETINDEX,0,0);
						//ȡ��ѡ����Ŀ���ı�����
						len = ::SendDlgItemMessage(_hSelf,IDC_RESULTLIST,LB_GETTEXTLEN,(WPARAM)index,0);
						if(len == LB_ERR){
							return FALSE;
						}
						//�½����Ա���ѡ���ı��Ļ�����
						TCHAR* T = new TCHAR[len+1];
						//ȡ��ѡ����Ŀ���ı�
						if(::SendDlgItemMessage(_hSelf,IDC_RESULTLIST,LB_GETTEXT,(WPARAM)index,(LPARAM)T) == LB_ERR){
							delete[] T;
							return FALSE;
						}
						//�ı���β
						T[len] = '\0';
						//ȡ���ı��е��кţ�����]:���͡�,���м���ı�
						TCHAR* start = wcschr(T,']');
						
						TCHAR* end = wcschr(T,',');
						if((start>T)&&(end>start)){
							len = end-start-1;
							TCHAR* line = lstrcpyn(T,start+2,len);
							__try{
								//תΪ����
								int lineNum = _wtoi(line);
								if (lineNum >0 )
								{
									// Get the current scintilla
									int which = -1;
									::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
									if (which == -1)
										return FALSE;
									HWND curScintilla = (which == 0)?nppData._scintillaMainHandle:nppData._scintillaSecondHandle;
									//������
									::SendMessage(curScintilla, SCI_ENSUREVISIBLE, lineNum-1, 0);
									::SendMessage(curScintilla, SCI_GOTOLINE, lineNum-1, 0);
								}
							}
							__except(EXCEPTION_EXECUTE_HANDLER){
								//���������Ϣ
								//OutputDebugString(str);
							}
						}
						//�ͷſռ�
						delete[] T;
						return TRUE;
					}
				}
			}
			return FALSE;
		}
		//���ڳߴ�仯ʱ����
		case WM_SIZE:
		{
			//ȡ�ÿ��
			static UINT cx,cy;
			cx = LOWORD(lParam);
			cy = HIWORD(lParam);
			if((cx>30)&&(cy>100)){
				//�����ؼ�λ�úͿ��
				::SetWindowPos(::GetDlgItem(_hSelf,IDC_RESULTLIST),0,0,30,cx,cy-30,SWP_NOOWNERZORDER);
				//SetWindowPos(GetDlgItem(_hSelf,IDOK),0,cx-75,3,70,24,SWP_NOOWNERZORDER);
			}
			return TRUE;
		}

		default :
			return DockingDlgInterface::run_dlgProc(message, wParam, lParam);
	}
}

