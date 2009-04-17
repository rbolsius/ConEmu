
/*
Copyright (c) 2009-2010 Maximus5
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifdef _DEBUG
//  �����������������, ����� ����� ����� ������� �������� (conemuc.exe) �������� MessageBox, ����� ����������� ����������
//  #define SHOW_STARTED_MSGBOX
//  #define SHOW_COMSPEC_STARTED_MSGBOX
//  #define SHOW_STARTED_ASSERT
#elif defined(__GNUC__)
//  �����������������, ����� ����� ����� ������� �������� (conemuc.exe) �������� MessageBox, ����� ����������� ����������
//  #define SHOW_STARTED_MSGBOX
#else
//
#endif

#include "ConEmuC.h"

WARNING("����������� ����� ������� ������� SetForegroundWindow �� GUI ����, ���� � ������ �������");
WARNING("����������� �������� ��� � ��� ������������� ��������");

WARNING("��� ������� ��� ComSpec �������� ������: {crNewSize.X>=MIN_CON_WIDTH && crNewSize.Y>=MIN_CON_HEIGHT}");
//E:\Source\FARUnicode\trunk\unicode_far\Debug.32.vc\ConEmuC.exe /c tools\gawk.exe -f .\scripts\gendate.awk


#ifdef _DEBUG
wchar_t gszDbgModLabel[6] = {0};
#endif

WARNING("!!!! ���� ����� ��� ��������� ������� ���������� ������� ���");
// � ��������� ��� � RefreshThread. ���� �� �� 0 - � ������ ������ (100��?)
// �� ������������� ���������� ������� � �������� ��� � 0.


WARNING("�������� ���-�� ����� ����������� ������������� ������ ����������� �������, � �� ������ �� �������");

WARNING("����� ������ ����� ������������ �������� ������� GUI ���� (���� ��� ����). ���� ����� ���� ������� �� far, � CMD.exe");

WARNING("���� GUI ����, ��� �� ���������� �� �������� - �������� ���������� ���� � �������� ���������� ����� ������");

WARNING("� ��������� ������� �� ����������� �� EVENT_CONSOLE_UPDATE_SIMPLE �� EVENT_CONSOLE_UPDATE_REGION");
// ������. ��������� cmd.exe. �������� �����-�� ����� � ��������� ������ � �������� 'Esc'
// ��� Esc ������� ������� ������ �� ���������, � ����� � ������� ���������!



FGetConsoleKeyboardLayoutName pfnGetConsoleKeyboardLayoutName = NULL;
FGetConsoleProcessList pfnGetConsoleProcessList = NULL;
FDebugActiveProcessStop pfnDebugActiveProcessStop = NULL;
FDebugSetProcessKillOnExit pfnDebugSetProcessKillOnExit = NULL;


/* Console Handles */
//MConHandle ghConIn ( L"CONIN$" );
MConHandle ghConOut ( L"CONOUT$" );


/*  Global  */
DWORD   gnSelfPID = 0;
//HANDLE  ghConIn = NULL, ghConOut = NULL;
HWND    ghConWnd = NULL;
HWND    ghConEmuWnd = NULL; // Root! window
HANDLE  ghExitQueryEvent = NULL;
HANDLE  ghQuitEvent = NULL;
bool    gbQuit = false;
BOOL	gbInShutdown = FALSE;
int     gnConfirmExitParm = 0; // 1 - CONFIRM, 2 - NOCONFIRM
BOOL    gbAlwaysConfirmExit = FALSE;
BOOL	gbAutoDisableConfirmExit = FALSE; // ���� �������� ������� ���������� ���������� (10 ���) - ����� ������� gbAlwaysConfirmExit
BOOL    gbRootAliveLess10sec = FALSE; // �������� ������� ���������� ����� CHECK_ROOTOK_TIMEOUT
int     gbRootWasFoundInCon = 0;
BOOL    gbAttachMode = FALSE;
BOOL    gbForceHideConWnd = FALSE;
DWORD   gdwMainThreadId = 0;
//int       gnBufferHeight = 0;
wchar_t* gpszRunCmd = NULL;
DWORD   gnImageSubsystem = 0;
//HANDLE  ghCtrlCEvent = NULL, ghCtrlBreakEvent = NULL;
HANDLE ghHeap = NULL; //HeapCreate(HEAP_GENERATE_EXCEPTIONS, nMinHeapSize, 0);
#ifdef _DEBUG
size_t gnHeapUsed = 0, gnHeapMax = 0;
HANDLE ghFarInExecuteEvent;
#endif

RunMode gnRunMode = RM_UNDEFINED;

BOOL gbNoCreateProcess = FALSE;
BOOL gbDebugProcess = FALSE;
int  gnCmdUnicodeMode = 0;
BOOL gbRootIsCmdExe = TRUE;
OSVERSIONINFO gOSVer;


SrvInfo srv = {0};

#pragma pack(push, 1)
CESERVER_CONSAVE* gpStoredOutput = NULL;
#pragma pack(pop)

CmdInfo cmd = {0};

COORD gcrBufferSize = {80,25};
BOOL  gbParmBufferSize = FALSE;
SHORT gnBufferHeight = 0;
wchar_t* gpszPrevConTitle = NULL;

HANDLE ghLogSize = NULL;
wchar_t* wpszLogSizeFile = NULL;


BOOL gbInRecreateRoot = FALSE;




// 04.12.2009 Maks + "__cdecl"
int __cdecl main()
{
	TODO("����� ��� ������� �������� �������, �������������� �������� 80x25 � ��������� ������� �����");

	//#ifdef _DEBUG
	//InitializeCriticalSection(&gcsHeap);
	//#endif

	int iRc = 100;
	PROCESS_INFORMATION pi; memset(&pi, 0, sizeof(pi));
	STARTUPINFOW si; memset(&si, 0, sizeof(si)); si.cb = sizeof(si);
	DWORD dwErr = 0, nWait = 0;
	BOOL lbRc = FALSE;
	DWORD mode = 0;
	//BOOL lb = FALSE;

	ghHeap = HeapCreate(HEAP_GENERATE_EXCEPTIONS, 200000, 0);
	memset(&gOSVer, 0, sizeof(gOSVer));
	gOSVer.dwOSVersionInfoSize = sizeof(gOSVer);
	GetVersionEx(&gOSVer);
	
	gpNullSecurity = NullSecurity();
	
	HMODULE hKernel = GetModuleHandleW (L"kernel32.dll");
	
	if (hKernel) {
		pfnGetConsoleKeyboardLayoutName = (FGetConsoleKeyboardLayoutName)GetProcAddress (hKernel, "GetConsoleKeyboardLayoutNameW");
		pfnGetConsoleProcessList = (FGetConsoleProcessList)GetProcAddress (hKernel, "GetConsoleProcessList");
	}
	

	// ����� ����������� ����
	ghConWnd = GetConsoleWindow();
	_ASSERTE(ghConWnd!=NULL);
	if (!ghConWnd) {
		dwErr = GetLastError();
		_printf("ghConWnd==NULL, ErrCode=0x%08X\n", dwErr);
		iRc = CERR_GETCONSOLEWINDOW; goto wrap;
	}
	// PID
	gnSelfPID = GetCurrentProcessId();
	gdwMainThreadId = GetCurrentThreadId();

#ifdef _DEBUG
	// ��� ������� ��������� � ���������� (���� ������������) ������ ����
	wchar_t szEvtName[64]; wsprintf(szEvtName, L"FARconEXEC:%08X", (DWORD)ghConWnd);
	ghFarInExecuteEvent = CreateEvent(0, TRUE, FALSE, szEvtName);
#endif

	
#if defined(SHOW_STARTED_MSGBOX) || defined(SHOW_COMSPEC_STARTED_MSGBOX)
	if (!IsDebuggerPresent()) {
		wchar_t szTitle[100]; wsprintf(szTitle, L"ConEmuC Loaded (PID=%i)", gnSelfPID);
		const wchar_t* pszCmdLine = GetCommandLineW();
		MessageBox(NULL,pszCmdLine,szTitle,0);
	}
#endif
#ifdef SHOW_STARTED_ASSERT
	if (!IsDebuggerPresent()) {
		_ASSERT(FALSE);
	}
#endif

	PRINT_COMSPEC(L"ConEmuC started: %s\n", GetCommandLineW());
	
	if ((iRc = ParseCommandLine(GetCommandLineW(), &gpszRunCmd)) != 0)
		goto wrap;
	//#ifdef _DEBUG
	//CreateLogSizeFile();
	//#endif
	
	/* ***************************** */
	/* *** "�����" ������������� *** */
	/* ***************************** */
	
	
	// ������� ������������ ��� ���� �������
	ghExitQueryEvent = CreateEvent(NULL, TRUE/*������������ � ���������� �����, manual*/, FALSE, NULL);
	if (!ghExitQueryEvent) {
		dwErr = GetLastError();
		_printf("CreateEvent() failed, ErrCode=0x%08X\n", dwErr); 
		iRc = CERR_EXITEVENT; goto wrap;
	}
	ResetEvent(ghExitQueryEvent);
	ghQuitEvent = CreateEvent(NULL, TRUE/*������������ � ���������� �����, manual*/, FALSE, NULL);
	if (!ghQuitEvent) {
		dwErr = GetLastError();
		_printf("CreateEvent() failed, ErrCode=0x%08X\n", dwErr); 
		iRc = CERR_EXITEVENT; goto wrap;
	}
	ResetEvent(ghQuitEvent);

	// �����������
	//ghConIn  = CreateFile(L"CONIN$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_READ,
	//            0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	//if ((HANDLE)ghConIn == INVALID_HANDLE_VALUE) {
	//    dwErr = GetLastError();
	//    _printf("CreateFile(CONIN$) failed, ErrCode=0x%08X\n", dwErr); 
	//    iRc = CERR_CONINFAILED; goto wrap;
	//}
	// �����������
	//ghConOut = CreateFile(L"CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_READ,
	//            0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (gnRunMode == RM_SERVER) {
		if ((HANDLE)ghConOut == INVALID_HANDLE_VALUE) {
			dwErr = GetLastError();
			_printf("CreateFile(CONOUT$) failed, ErrCode=0x%08X\n", dwErr); 
			iRc = CERR_CONOUTFAILED; goto wrap;
		}
	}
	
	//2009-05-30 ��������� ��� ����� ?
	//SetHandleInformation(GetStdHandle(STD_INPUT_HANDLE), HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
	//SetHandleInformation(GetStdHandle(STD_OUTPUT_HANDLE), HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
	//SetHandleInformation(GetStdHandle(STD_ERROR_HANDLE), HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
	
	mode = 0;
	/*lb = GetConsoleMode(ghConIn, &mode);
	if (!(mode & ENABLE_MOUSE_INPUT)) {
		mode |= ENABLE_MOUSE_INPUT;
		lb = SetConsoleMode(ghConIn, mode);
	}*/

	// �����������, ����� �� CtrlC �� ��������
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)HandlerRoutine, true);
	//SetConsoleMode(ghConIn, 0);

	/* ******************************** */
	/* *** "��������" ������������� *** */
	/* ******************************** */
	if (gnRunMode == RM_SERVER) {
		if ((iRc = ServerInit()) != 0)
			goto wrap;
	} else {
		if ((iRc = ComspecInit()) != 0)
			goto wrap;
	}

	
	
	
	/* ********************************* */
	/* *** ������ ��������� �������� *** */
	/* ********************************* */
	
	// CREATE_NEW_PROCESS_GROUP - ����, ��������� �������� Ctrl-C
	// ����� CreateProcessW ����� ������� 0, ����� ��-�� ����������� ����� ���������
	// timeout �������� ��������� �������� ��� �� ������ �� CreateProcessW
	if (!gbAttachMode)
		srv.nProcessStartTick = 0;
	if (gbNoCreateProcess) {
		lbRc = TRUE; // ������� ��� �������, ������ ��������� � ConEmu (GUI)
		pi.hProcess = srv.hRootProcess;
		pi.dwProcessId = srv.dwRootProcess;
	} else {

		#ifdef _DEBUG
		if (ghFarInExecuteEvent && wcsstr(gpszRunCmd,L"far.exe"))
			ResetEvent(ghFarInExecuteEvent);
		#endif
		
		lbRc = CreateProcessW(NULL, gpszRunCmd, NULL,NULL, TRUE, 
				NORMAL_PRIORITY_CLASS/*|CREATE_NEW_PROCESS_GROUP*/
				|((gnRunMode == RM_SERVER) ? CREATE_SUSPENDED : 0), 
				NULL, NULL, &si, &pi);
		dwErr = GetLastError();
		if (lbRc && (gnRunMode == RM_SERVER)) {
			TODO("Injects");
		
			// ��������� �������
			ResumeThread(pi.hThread);
		}
		if (!lbRc && dwErr == 0x000002E4) {
			// ��������� ������ � ������ comspec - ����� ���������� ����� �������
			_ASSERTE(gnRunMode != RM_SERVER);
			PRINT_COMSPEC(L"Vista+: The requested operation requires elevation (ErrCode=0x%08X).\n", dwErr);
			// Vista: The requested operation requires elevation.
			LPCWSTR pszCmd = gpszRunCmd;
			wchar_t szVerb[10], szExec[MAX_PATH+1];
			if (NextArg(&pszCmd, szExec) == 0) {
				SHELLEXECUTEINFO sei = {sizeof(SHELLEXECUTEINFO)};
				sei.hwnd = ghConEmuWnd;
				sei.fMask = SEE_MASK_NO_CONSOLE; //SEE_MASK_NOCLOSEPROCESS; -- ������ ����� ���������� ��� - ������� ����������� � ����� �������
				sei.lpVerb = wcscpy(szVerb, L"open");
				sei.lpFile = szExec;
				sei.lpParameters = pszCmd;
				sei.nShow = SW_SHOWNORMAL;
				if ((lbRc = ShellExecuteEx(&sei)) == FALSE) {
					dwErr = GetLastError();
				} else {
					// OK
					//pi.hProcess = sei.hProcess;
					//typedef DWORD (WINAPI* FGetProcessId)(HANDLE);
					//FGetProcessId fGetProcessId = NULL;
					//HMODULE hKernel = GetModuleHandle(L"kernel32.dll");
					//if (hKernel)
					//	fGetProcessId = (FGetProcessId)GetProcAddress(hKernel, "GetProcessId");
					//if (fGetProcessId) {
					//	pi.dwProcessId = fGetProcessId(sei.hProcess);
					//} else {
					//	// ��� ������� �����������, �� ��������� ������ 0x000002E4 ����� ������ � Vista,
					//	// � ��� ���� ������� GetProcessId, ��� ��� ��� ����� �������������� �� ������
					//	_ASSERTE(fGetProcessId!=NULL);
					//	pi.dwProcessId = GetCurrentProcessId();
					//}
					pi.hProcess = NULL; pi.dwProcessId = 0;
					pi.hThread = NULL; pi.dwThreadId = 0;
					// �.�. ����������� ����� ������� - ������������� �� �������� ���� ����� �� �����
					DisableAutoConfirmExit();
					iRc = 0; goto wrap;
				}
			}
		}
	}
	if (!lbRc)
	{
		wchar_t* lpMsgBuf = NULL;
		DWORD nFmtRc, nFmtErr = 0;
		if (dwErr == 5) {
			lpMsgBuf = (wchar_t*)LocalAlloc(LPTR, 255);
			wcscpy(lpMsgBuf, L"Access is denied.\nThis may be cause of antiviral or file permissions denial.");
		} else {
			nFmtRc = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
				NULL, dwErr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&lpMsgBuf, 0, NULL );
			if (!nFmtRc)
				nFmtErr = GetLastError();
		}
		
		_printf("Can't create process, ErrCode=0x%08X, Description:\n", dwErr);
		_wprintf((lpMsgBuf == NULL) ? L"<Unknown error>" : lpMsgBuf);
		_printf("\nCommand to be executed:\n");
		_wprintf(gpszRunCmd);
		_printf("\n");
		
		if (lpMsgBuf) LocalFree(lpMsgBuf);
		iRc = CERR_CREATEPROCESS; goto wrap;
	}
	if (gbAttachMode) {
		// �� ��������� � ��� ������������� ��������: 
		// ����� �� ��� ������� ��� ������ dos-������� � ����� ������� ����� -new_console
		// � ��������� ������ ���������� ������������� �������� ���������� �����������
		// -- DisableAutoConfirmExit(); - ����
	} else {
		srv.nProcessStartTick = GetTickCount();
	}
	//delete psNewCmd; psNewCmd = NULL;
	AllowSetForegroundWindow(pi.dwProcessId);


	
	/* ************************ */
	/* *** �������� ������� *** */
	/* ************************ */
	
	if (gnRunMode == RM_SERVER) {
		srv.hRootProcess  = pi.hProcess; pi.hProcess = NULL; // Required for Win2k
		srv.hRootThread   = pi.hThread;  pi.hThread  = NULL;
		srv.dwRootProcess = pi.dwProcessId;
		srv.dwRootThread  = pi.dwThreadId;
		srv.dwRootStartTime = GetTickCount();

		// ������ ����� ������� � ���������� ������ ��� �����
		CheckProcessCount(TRUE);
		#ifdef _DEBUG
		if (srv.nProcessCount && !srv.bDebuggerActive) {
			_ASSERTE(srv.pnProcesses[srv.nProcessCount-1]!=0);
		}
		#endif

		//if (pi.hProcess) SafeCloseHandle(pi.hProcess); 
		//if (pi.hThread) SafeCloseHandle(pi.hThread);

		if (srv.hConEmuGuiAttached) {
			DWORD dwWaitGui = WaitForSingleObject(srv.hConEmuGuiAttached, 1000);
			if (dwWaitGui == WAIT_OBJECT_0) {
				// GUI ���� �����
				wsprintf(srv.szGuiPipeName, CEGUIPIPENAME, L".", (DWORD)ghConWnd); // ��� gnSelfPID
			}
		}
	
		// ����, ���� � ������� �� ��������� ��������� (����� ������)
		TODO("���������, ����� �� ��� ����������, ��� CreateProcess ������, � � ������� �� �� ����������? �����, ���� ������� GUI");
		nWait = WaitForSingleObject(ghExitQueryEvent, CHECK_ANTIVIRUS_TIMEOUT); //������ �������� �������� ����� ��������� ���������
		if (nWait != WAIT_OBJECT_0) { // ���� �������
			iRc = srv.nProcessCount;
			// � ��������� � ������� ��� ��� ���
			if (iRc == 1 && !srv.bDebuggerActive) {
				_printf ("Process was not attached to console. Is it GUI?\nCommand to be executed:\n");
				_wprintf (gpszRunCmd);
				_printf ("\n");
				iRc = CERR_PROCESSTIMEOUT; goto wrap;
			}
		}
	} else {
		// � ������ ComSpec ��� ���������� ���������� ������ ��������� ��������

		//wchar_t szEvtName[128];
		//
		//wsprintf(szEvtName, CESIGNAL_C, pi.dwProcessId);
		//ghCtrlCEvent = CreateEvent(NULL, FALSE, FALSE, szEvtName);
		//wsprintf(szEvtName, CESIGNAL_BREAK, pi.dwProcessId);
		//ghCtrlBreakEvent = CreateEvent(NULL, FALSE, FALSE, szEvtName);
	}

	/* *************************** */
	/* *** �������� ���������� *** */
	/* *************************** */
wait:
	if (gnRunMode == RM_SERVER) {
		// �� ������� ���� ���� ������� � ������� ����������. ���� ���� � ������� �� ��������� ������ ����� ���
		nWait = WAIT_TIMEOUT;
		if (!srv.bDebuggerActive) {
			while (nWait == WAIT_TIMEOUT) {
				nWait = WaitForSingleObject(ghExitQueryEvent, 10);
			}
		} else {
			while (nWait == WAIT_TIMEOUT) {
				ProcessDebugEvent();
				nWait = WaitForSingleObject(ghExitQueryEvent, 0);
			}
			//gbAlwaysConfirmExit = TRUE;
		}
		#ifdef _DEBUG
		if (nWait == WAIT_OBJECT_0) {
			DEBUGSTR(L"*** FinilizeEvent was set!\n");
		}
		#endif
	} else {
		//HANDLE hEvents[3];
		//hEvents[0] = pi.hProcess;
		//hEvents[1] = ghCtrlCEvent;
		//hEvents[2] = ghCtrlBreakEvent;
		//WaitForSingleObject(pi.hProcess, INFINITE);
		DWORD dwWait = 0;
		dwWait = WaitForSingleObject(pi.hProcess, INFINITE);
		// �������� ExitCode
		GetExitCodeProcess(pi.hProcess, &cmd.nExitCode);

		// ����� ������� ������
		if (pi.hProcess) SafeCloseHandle(pi.hProcess); 
		if (pi.hThread) SafeCloseHandle(pi.hThread);
	}
	
	
	
	/* ************************* */
	/* *** ���������� ������ *** */
	/* ************************* */
	
	iRc = 0;
wrap:
	// � ���������, HandlerRoutine ����� ���� ��� �� ������, �������
	// � ����� ��������� ExitWaitForKey ��������� �������� ����� gbInShutdown
	PRINT_COMSPEC(L"Finalizing. gbInShutdown=%i\n", gbInShutdown);
	#ifdef SHOW_STARTED_MSGBOX
	MessageBox(GetConsoleWindow(), L"Finalizing", (gnRunMode == RM_SERVER) ? L"ConEmuC.Server" : L"ConEmuC.ComSpec", 0);
	#endif
	if (!gbInShutdown // ������ ���� ���� �� ����� ������� � ��������� ����
		&& ((iRc!=0 && iRc!=CERR_RUNNEWCONSOLE && iRc!=CERR_EMPTY_COMSPEC_CMDLINE) || gbAlwaysConfirmExit)
		)
	{
		BOOL lbProcessesLeft = FALSE, lbDontShowConsole = FALSE;
		if (pfnGetConsoleProcessList) {
			DWORD nProcesses[10];
			DWORD nProcCount = pfnGetConsoleProcessList ( nProcesses, 10 );
			if (nProcCount > 1)
				lbProcessesLeft = TRUE;
		}

		LPCWSTR pszMsg = NULL;
		if (lbProcessesLeft) {
			pszMsg = L"\n\nPress Enter to exit...";
			lbDontShowConsole = gnRunMode != RM_SERVER;
		} else {
			if (gbRootWasFoundInCon == 1) {
				if (gbRootAliveLess10sec) // �������� ������� ���������� ����� CHECK_ROOTOK_TIMEOUT
					pszMsg = L"\n\nConEmuC: Root process was alive less than 10 sec.\nPress Enter to close console...";
				else
					pszMsg = L"\n\nPress Enter to close console...";
			}
		}
		if (!pszMsg) // ����� - ��������� �� ���������
			pszMsg = L"\n\nPress Enter to close console, or wait...";

		ExitWaitForKey(VK_RETURN, pszMsg, TRUE, lbDontShowConsole);
		if (iRc == CERR_PROCESSTIMEOUT) {
			int nCount = srv.nProcessCount;
			if (nCount > 1 || srv.bDebuggerActive) {
				// ������� ���� ����������!
				goto wait;
			}
		}
	}

	// �� ������ ������ - �������� �������
	if (ghExitQueryEvent) SetEvent(ghExitQueryEvent);
	
	// ���������� RefreshThread, InputThread, ServerThread
	if (ghQuitEvent) SetEvent(ghQuitEvent);
	
	
	/* ***************************** */
	/* *** "��������" ���������� *** */
	/* ***************************** */
	
	if (gnRunMode == RM_SERVER) {
		ServerDone(iRc);
		//MessageBox(0,L"Server done...",L"ConEmuC",0);
	} else {
		ComspecDone(iRc);
		//MessageBox(0,L"Comspec done...",L"ConEmuC",0);
	}

	
	/* ************************** */
	/* *** "�����" ���������� *** */
	/* ************************** */
	
	if (gpszPrevConTitle && ghConWnd) {
		SetConsoleTitleW(gpszPrevConTitle);
		Free(gpszPrevConTitle);
	}

	LogSize(NULL, "Shutdown");
	
	//ghConIn.Close();
	ghConOut.Close();

	SafeCloseHandle(ghLogSize);
	if (wpszLogSizeFile) {
		//DeleteFile(wpszLogSizeFile);
		Free(wpszLogSizeFile); wpszLogSizeFile = NULL;
	}

#ifdef _DEBUG
	SafeCloseHandle(ghFarInExecuteEvent);
#endif
	
	if (gpszRunCmd) { delete gpszRunCmd; gpszRunCmd = NULL; }
	
	CommonShutdown();

	if (ghHeap) {
		HeapDestroy(ghHeap);
		ghHeap = NULL;
	}

	// ���� ����� ComSpec - ������� ��� �������� �� ����������� ��������
	if (iRc == 0 && gnRunMode == RM_COMSPEC)
		iRc = cmd.nExitCode;

	#ifdef SHOW_STARTED_MSGBOX
	MessageBox(GetConsoleWindow(), L"Exiting", (gnRunMode == RM_SERVER) ? L"ConEmuC.Server" : L"ConEmuC.ComSpec", 0);
	#endif
		
	return iRc;
}

//#if defined(CRTSTARTUP)
//extern "C"{
//  BOOL WINAPI _DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
//};
//
//BOOL WINAPI mainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
//{
//  DllMain(hDll, dwReason, lpReserved);
//  return TRUE;
//}
//#endif


void Help()
{
	_printf(
		"ConEmuC. Copyright (c) 2009, Maximus5\n"
		"This is a console part of ConEmu product.\n"
		"Usage: ConEmuC [switches] [/U | /A] /C <command line, passed to %COMSPEC%>\n"
		"   or: ConEmuC [switches] /ROOT <program with arguments, far.exe for example>\n"
		"   or: ConEmuC /ATTACH /NOCMD\n"
		"   or: ConEmuC /?\n"
		"Switches:\n"
		"     /[NO]CONFIRM - [don't] confirm closing console on program termination\n"
		"     /ATTACH      - auto attach to ConEmu GUI\n"
		"     /NOCMD       - attach current (existing) console to GUI\n"
		"     /B{W|H|Z}    - define window width, height and buffer height\n"
		"     /F{N|W|H}    - define console font name, width, height\n"
		"     /LOG[N]      - create (debug) log file, N is number from 0 to 3\n"
	);
}

#pragma warning( push )
#pragma warning(disable : 6400)
BOOL IsExecutable(LPCWSTR aszFilePathName)
{
	#pragma warning( push )
	#pragma warning(disable : 6400)
	LPCWSTR pwszDot = wcsrchr(aszFilePathName, L'.');
	if (pwszDot) { // ���� ������ .exe ��� .com ����
		if (lstrcmpiW(pwszDot, L".exe")==0 || lstrcmpiW(pwszDot, L".com")==0) {
			if (FileExists(aszFilePathName))
				return TRUE;
		}
	}
	return FALSE;
}
#pragma warning( pop )

BOOL IsNeedCmd(LPCWSTR asCmdLine, BOOL *rbNeedCutStartEndQuot)
{
	_ASSERTE(asCmdLine && *asCmdLine);

	gbRootIsCmdExe = TRUE;

	if (!asCmdLine || *asCmdLine == 0)
		return TRUE;

	// ���� ���� ���� �� ������ ���������������, ��� ������� - ����� CMD.EXE
	if (wcschr(asCmdLine, L'&') || 
		wcschr(asCmdLine, L'>') || 
		wcschr(asCmdLine, L'<') || 
		wcschr(asCmdLine, L'|') ||
		wcschr(asCmdLine, L'^') // ��� �������������
		)
	{
		return TRUE;
	}

	wchar_t szArg[MAX_PATH+10] = {0};
	int iRc = 0;
	BOOL lbFirstWasGot = FALSE;
	LPCWSTR pwszCopy = asCmdLine;

	// cmd /c ""c:\program files\arc\7z.exe" -?"   // �� ��� � ������ ����� ���� ��������...
	// cmd /c "dir c:\"
	int nLastChar = lstrlenW(pwszCopy) - 1;
	if (pwszCopy[0] == L'"' && pwszCopy[nLastChar] == L'"') {
		if (pwszCopy[1] == L'"' && pwszCopy[2]) {
			pwszCopy ++; // ��������� ������ ������� � �������� ����: ""c:\program files\arc\7z.exe" -?"
			if (rbNeedCutStartEndQuot) *rbNeedCutStartEndQuot = TRUE;
		} else
		// ������� �� ""F:\VCProject\FarPlugin\#FAR180\far.exe  -new_console""
		//if (wcschr(pwszCopy+1, L'"') == (pwszCopy+nLastChar)) {
		//	LPCWSTR pwszTemp = pwszCopy;
		//	// ������� ������ ������� (����������� ����?)
		//	if ((iRc = NextArg(&pwszTemp, szArg)) != 0) {
		//		//Parsing command line failed
		//		return TRUE;
		//	}
		//	pwszCopy ++; // ��������� ������ ������� � �������� ����: "c:\arc\7z.exe -?"
		//	lbFirstWasGot = TRUE;
		//	if (rbNeedCutStartEndQuot) *rbNeedCutStartEndQuot = TRUE;
		//} else
		{
			// ��������� ������ ������� �: "C:\GCC\msys\bin\make.EXE -f "makefile" COMMON="../../../plugins/common""
			LPCWSTR pwszTemp = pwszCopy + 1;
			// ������� ������ ������� (����������� ����?)
			if ((iRc = NextArg(&pwszTemp, szArg)) != 0) {
				//Parsing command line failed
				return TRUE;
			}
			LPCWSTR pwszQ = pwszCopy + 1 + wcslen(szArg);
			if (*pwszQ != L'"' && IsExecutable(szArg)) {
				pwszCopy ++; // �����������
				lbFirstWasGot = TRUE;
				if (rbNeedCutStartEndQuot) *rbNeedCutStartEndQuot = TRUE;
			}
		}
	}

	// ������� ������ ������� (����������� ����?)
	if (!lbFirstWasGot) {
		if ((iRc = NextArg(&pwszCopy, szArg)) != 0) {
			//Parsing command line failed
			return TRUE;
		}
	}
	//pwszCopy = wcsrchr(szArg, L'\\'); if (!pwszCopy) pwszCopy = szArg; else pwszCopy ++;
	pwszCopy = PointToName(szArg);
	//2009-08-27
	wchar_t *pwszEndSpace = szArg + lstrlenW(szArg) - 1;
	while (*pwszEndSpace == L' ' && pwszEndSpace > szArg)
		*(pwszEndSpace--) = 0;

	#pragma warning( push )
	#pragma warning(disable : 6400)

	if (lstrcmpiW(pwszCopy, L"cmd")==0 || lstrcmpiW(pwszCopy, L"cmd.exe")==0)
	{
		gbRootIsCmdExe = TRUE; // ��� ������ ���� ���������, �� ��������
		gbAlwaysConfirmExit = TRUE; gbAutoDisableConfirmExit = FALSE;
		return FALSE; // ��� ������ ��������� ���������, cmd.exe � ������ ��������� �� �����
	}

	if (lstrcmpiW(pwszCopy, L"far")==0 || lstrcmpiW(pwszCopy, L"far.exe")==0)
	{
		gbAutoDisableConfirmExit = TRUE;
		gbRootIsCmdExe = FALSE; // FAR!
		return FALSE; // ��� ������ ��������� ���������, cmd.exe � ������ ��������� �� �����
	}

	if (IsExecutable(szArg)) {
		gbRootIsCmdExe = FALSE; // ��� ������ �������� - ����� �� ��������
		return FALSE; // ����������� ���������� ���������� ���������. cmd.exe �� ���������
	}

	//����� ��� �������� ������ �: SearchPath, GetFullPathName, ������� ���������� .exe & .com
	//���� ��� ��� ��������� ������ ���� � ��������, ��� ��� ����� �� ��������������

	gbRootIsCmdExe = TRUE;
	#pragma warning( pop )
	return TRUE;
}

BOOL FileExists(LPCWSTR asFile)
{
	WIN32_FIND_DATA fnd; memset(&fnd, 0, sizeof(fnd));
	HANDLE h = FindFirstFile(asFile, &fnd);
	if (h != INVALID_HANDLE_VALUE) {
		FindClose(h);
		return (fnd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
	}
	return FALSE;
}

void CheckUnicodeMode()
{
	if (gnCmdUnicodeMode) return;
	wchar_t szValue[16] = {0};
	if (GetEnvironmentVariable(L"ConEmuOutput", szValue, sizeof(szValue)/sizeof(szValue[0]))) {
		if (lstrcmpi(szValue, L"UNICODE") == 0)
			gnCmdUnicodeMode = 2;
		else if (lstrcmpi(szValue, L"ANSI") == 0)
			gnCmdUnicodeMode = 1;
	}
}

// ������ ���������� ��������� ������
int ParseCommandLine(LPCWSTR asCmdLine, wchar_t** psNewCmd)
{
	int iRc = 0;
	wchar_t szArg[MAX_PATH+1] = {0};
	LPCWSTR pszArgStarts = NULL;
	wchar_t szComSpec[MAX_PATH+1] = {0};
	LPCWSTR pwszCopy = NULL;
	wchar_t* psFilePart = NULL;
	BOOL bViaCmdExe = TRUE;
	gbRootIsCmdExe = TRUE;
	size_t nCmdLine = 0;
	LPCWSTR pwszStartCmdLine = asCmdLine;
	BOOL lbNeedCutStartEndQuot = FALSE;
	
	if (!asCmdLine || !*asCmdLine)
	{
		DWORD dwErr = GetLastError();
		_printf("GetCommandLineW failed! ErrCode=0x%08X\n", dwErr);
		return CERR_GETCOMMANDLINE;
	}

	gnRunMode = RM_UNDEFINED;
	
	
	while ((iRc = NextArg(&asCmdLine, szArg, &pszArgStarts)) == 0)
	{
		if ((szArg[0] == L'/' || szArg[0] == L'-')
			&& (szArg[1] == L'?' || ((szArg[1] & ~0x20) == L'H'))
			&& szArg[2] == 0)
		{
			Help();
			return CERR_HELPREQUESTED;
		}

		// ����� - ��������� ����� � ��������� ��� "/"
		if (szArg[0] != L'/')
			continue;
		
		if (wcscmp(szArg, L"/CONFIRM")==0) {
			TODO("��������, ��� ����� � gbAutoDisableConfirmExit");
			gnConfirmExitParm = 1;
			gbAlwaysConfirmExit = TRUE; gbAutoDisableConfirmExit = TRUE;
		} else
		if (wcscmp(szArg, L"/NOCONFIRM")==0) {
			gnConfirmExitParm = 2;
			gbAlwaysConfirmExit = FALSE; gbAutoDisableConfirmExit = FALSE;
		} else

		if (wcscmp(szArg, L"/ATTACH")==0) {
			gbAttachMode = TRUE;
			gnRunMode = RM_SERVER;
		} else

		if (wcscmp(szArg, L"/HIDE")==0) {
			gbForceHideConWnd = TRUE;
		} else

		if (wcsncmp(szArg, L"/B", 2)==0) {
			wchar_t* pszEnd = NULL;
			if (wcsncmp(szArg, L"/BW=", 4)==0) {
				gcrBufferSize.X = /*_wtoi(szArg+4);*/(SHORT)wcstol(szArg+4,&pszEnd,10); gbParmBufferSize = TRUE;
			} else if (wcsncmp(szArg, L"/BH=", 4)==0) {
				gcrBufferSize.Y = /*_wtoi(szArg+4);*/(SHORT)wcstol(szArg+4,&pszEnd,10); gbParmBufferSize = TRUE;
			} else if (wcsncmp(szArg, L"/BZ=", 4)==0) {
				gnBufferHeight = /*_wtoi(szArg+4);*/(SHORT)wcstol(szArg+4,&pszEnd,10); gbParmBufferSize = TRUE;
			}
		} else

		if (wcsncmp(szArg, L"/F", 2)==0) {
			wchar_t* pszEnd = NULL;
			if (wcsncmp(szArg, L"/FN=", 4)==0) {
				lstrcpynW(srv.szConsoleFont, szArg+4, 32);
			} else if (wcsncmp(szArg, L"/FW=", 4)==0) {
				srv.nConFontWidth = /*_wtoi(szArg+4);*/(SHORT)wcstol(szArg+4,&pszEnd,10);
			} else if (wcsncmp(szArg, L"/FH=", 4)==0) {
				srv.nConFontHeight = /*_wtoi(szArg+4);*/(SHORT)wcstol(szArg+4,&pszEnd,10);
			//} else if (wcsncmp(szArg, L"/FF=", 4)==0) {
			//  lstrcpynW(srv.szConsoleFontFile, szArg+4, MAX_PATH);
			}
		} else
		
		if (wcsncmp(szArg, L"/LOG",4)==0) {
			int nLevel = 0;
			if (szArg[4]==L'1') nLevel = 1; else if (szArg[4]>=L'2') nLevel = 2;
			CreateLogSizeFile(nLevel);
		} else
		
		if (wcscmp(szArg, L"/NOCMD")==0) {
			gnRunMode = RM_SERVER;
			gbNoCreateProcess = TRUE;
		} else

		if (wcsncmp(szArg, L"/GID=", 5)==0) {
			gnRunMode = RM_SERVER;
			wchar_t* pszEnd = NULL;
			srv.dwGuiPID = wcstol(szArg+5, &pszEnd, 10);
			if (srv.dwGuiPID == 0) {
				_printf ("Invalid GUI PID specified:\n");
				_wprintf (GetCommandLineW());
				_printf ("\n");
				return CERR_CARGUMENT;
			}
		} else
		
		if (wcsncmp(szArg, L"/PID=", 5)==0) {
			gnRunMode = RM_SERVER;
			gbNoCreateProcess = TRUE;
			wchar_t* pszEnd = NULL;
			//srv.dwRootProcess = _wtol(szArg+5);
			srv.dwRootProcess = wcstol(szArg+5, &pszEnd, 10);
			if (srv.dwRootProcess == 0) {
				_printf ("Attach to GUI was requested, but invalid PID specified:\n");
				_wprintf (GetCommandLineW());
				_printf ("\n");
				return CERR_CARGUMENT;
			}
		} else

		if (wcsncmp(szArg, L"/DEBUGPID=", 10)==0) {
			gnRunMode = RM_SERVER;
			gbNoCreateProcess = gbDebugProcess = TRUE;
			wchar_t* pszEnd = NULL;
			//srv.dwRootProcess = _wtol(szArg+10);
			srv.dwRootProcess = wcstol(szArg+10, &pszEnd, 10);
			if (srv.dwRootProcess == 0) {
				_printf ("Debug of process was requested, but invalid PID specified:\n");
				_wprintf (GetCommandLineW());
				_printf ("\n");
				return CERR_CARGUMENT;
			}
		} else
		
		if (wcscmp(szArg, L"/A")==0 || wcscmp(szArg, L"/a")==0) {
			gnCmdUnicodeMode = 1;
		} else
		if (wcscmp(szArg, L"/U")==0 || wcscmp(szArg, L"/u")==0) {
			gnCmdUnicodeMode = 2;
		} else
		
		// ����� ���� ���������� - ���� ��, ��� ���������� � CreateProcess!
		if (wcscmp(szArg, L"/ROOT")==0 || wcscmp(szArg, L"/root")==0) {
			gnRunMode = RM_SERVER; gbNoCreateProcess = FALSE;
			break; // asCmdLine ��� ��������� �� ����������� ���������
		} else

		// ����� ���� ���������� - ���� ��, ��� ���������� � COMSPEC (CreateProcess)!
		//if (wcscmp(szArg, L"/C")==0 || wcscmp(szArg, L"/c")==0 || wcscmp(szArg, L"/K")==0 || wcscmp(szArg, L"/k")==0) {
		if (szArg[0] == L'/' && (((szArg[1] & ~0x20) == L'C') || ((szArg[1] & ~0x20) == L'K'))) {
			gbNoCreateProcess = FALSE;
			if (szArg[2] == 0) // "/c" ��� "/k"
				gnRunMode = RM_COMSPEC;
			if (gnRunMode == RM_UNDEFINED && szArg[4] == 0
				&& ((szArg[2] & ~0x20) == L'M') && ((szArg[3] & ~0x20) == L'D'))
				gnRunMode = RM_SERVER;
			// ���� ��� ������ �� ��� ��� �� ���������� - ������� ��� ����� ComSpec
			// � ������� ���������� ����� ����� /c (����� ���� "cmd /cecho xxx")
			if (gnRunMode == RM_UNDEFINED) {
				gnRunMode = RM_COMSPEC;
				cmd.bK = (szArg[1] & ~0x20) == L'K';
				// ��������� ��������� ����������� "cmd /cecho xxx"
				asCmdLine = pszArgStarts + 2;
				while (*asCmdLine==L' ' || *asCmdLine==L'\t') asCmdLine++;
			}
			break; // asCmdLine ��� ��������� �� ����������� ���������
		}
	}
	
	if (gnRunMode == RM_SERVER) {
		if (gbDebugProcess) {
			if (!DebugActiveProcess(srv.dwRootProcess)) {
				DWORD dwErr = GetLastError();
				_printf("Can't start debugger! ErrCode=0x%08X\n", dwErr);
				return CERR_CANTSTARTDEBUGGER;
			}
			pfnDebugActiveProcessStop = (FDebugActiveProcessStop)GetProcAddress(GetModuleHandle(L"kernel32.dll"),"DebugActiveProcessStop");
			pfnDebugSetProcessKillOnExit = (FDebugSetProcessKillOnExit)GetProcAddress(GetModuleHandle(L"kernel32.dll"),"DebugSetProcessKillOnExit");
			if (pfnDebugSetProcessKillOnExit) pfnDebugSetProcessKillOnExit(FALSE/*KillOnExit*/);
			srv.bDebuggerActive = TRUE;
		
			wchar_t* pszNewCmd = new wchar_t[1];
			if (!pszNewCmd) {
				_printf ("Can't allocate 1 wchar!\n");
				return CERR_NOTENOUGHMEM1;
			}
			pszNewCmd[0] = 0;
			return 0;
		} else
		if (gbNoCreateProcess && gbAttachMode) {
			if (pfnGetConsoleProcessList==NULL) {
				_printf ("Attach to GUI was requested, but required WinXP or higher:\n");
				_wprintf (GetCommandLineW());
				_printf ("\n");
				return CERR_CARGUMENT;
			}
			DWORD nProcesses[10];
			DWORD nProcCount = pfnGetConsoleProcessList ( nProcesses, 10 );
			if (nProcCount < 2) {
				_printf ("Attach to GUI was requested, but there is no console processes:\n");
				_wprintf (GetCommandLineW());
				_printf ("\n");
				return CERR_CARGUMENT;
			}
			// ���� cmd.exe ������� �� cmd.exe (� ������� ��� ������ ���� ���������) - ������ �� ������
			if (nProcCount > 2) {
				// � �������� ������ ��� ����������
				wchar_t szProc[255] ={0}, szTmp[10]; //wsprintfW(szProc, L"%i, %i, %i", nProcesses[0], nProcesses[1], nProcesses[2]);
				for (DWORD n=0; n<nProcCount && n<20; n++) {
					if (n) lstrcatW(szProc, L", ");
					wsprintf(szTmp, L"%i", nProcesses[0]);
					//lstrcatW(szProc, _ltow(nProcesses[0], szTmp, 10));
					lstrcatW(szProc, szTmp);
				}
				PRINT_COMSPEC(L"Attach to GUI was requested, but there is more then 2 console processes: %s\n", szProc);
				return CERR_CARGUMENT;
			}

			wchar_t* pszNewCmd = new wchar_t[1];
			if (!pszNewCmd) {
				_printf ("Can't allocate 1 wchar!\n");
				return CERR_NOTENOUGHMEM1;
			}
			pszNewCmd[0] = 0;
			return 0;
		}
	}

	if (iRc != 0) {
		if (iRc == CERR_CMDLINEEMPTY) {
			Help();
			_printf ("\n\nParsing command line failed (/C argument not found):\n");
			_wprintf (GetCommandLineW());
			_printf ("\n");
		} else {
			_printf ("Parsing command line failed:\n");
			_wprintf (asCmdLine);
			_printf ("\n");
		}
		return iRc;
	}
	if (gnRunMode == RM_UNDEFINED) {
		_printf ("Parsing command line failed (/C argument not found):\n");
		_wprintf (GetCommandLineW());
		_printf ("\n");
		return CERR_CARGUMENT;
	}

	if (gnRunMode == RM_COMSPEC) {
	
		// ����� ������� ������� ����� �������?
		int nArgLen = lstrlenA(" -new_console");
		pwszCopy = (wchar_t*)wcsstr(asCmdLine, L" -new_console");
		// ���� ����� -new_console ���� ������, ��� ��� ������ ����� ������
		if (pwszCopy && 
			(pwszCopy[nArgLen]==L' ' || pwszCopy[nArgLen]==0
			 || (pwszCopy[nArgLen]==L'"' || pwszCopy[nArgLen+1]==0)))
		{
			// ����� ������������
			cmd.bNewConsole = TRUE;
			//
			size_t nNewLen = wcslen(pwszStartCmdLine) + 135;
			//
			BOOL lbIsNeedCmd = IsNeedCmd(asCmdLine, &lbNeedCutStartEndQuot);
			
			// Font, size, etc.
			
			CESERVER_REQ *pIn = NULL, *pOut = NULL;
			wchar_t* pszAddNewConArgs = NULL;
			if ((pIn = ExecuteNewCmd(CECMD_GETNEWCONPARM, sizeof(CESERVER_REQ_HDR)+2*sizeof(DWORD))) != NULL) {
				pIn->dwData[0] = gnSelfPID;
				pIn->dwData[1] = lbIsNeedCmd;
				
				PRINT_COMSPEC(L"Retrieve new console add args (begin)\n",0);
				pOut = ExecuteGuiCmd(ghConWnd, pIn, ghConWnd);
				PRINT_COMSPEC(L"Retrieve new console add args (begin)\n",0);
				
				if (pOut) {
					pszAddNewConArgs = (wchar_t*)pOut->Data;
					if (*pszAddNewConArgs == 0) {
						ExecuteFreeResult(pOut); pOut = NULL; pszAddNewConArgs = NULL;
					} else {
						nNewLen += wcslen(pszAddNewConArgs) + 1;
					}
				}
				ExecuteFreeResult(pIn); pIn = NULL;
			}
			//
			wchar_t* pszNewCmd = new wchar_t[nNewLen];
			if (!pszNewCmd) {
				_printf ("Can't allocate %i wchars!\n", (DWORD)nNewLen);
				return CERR_NOTENOUGHMEM1;
			}
			// ������� ����������� ���, ��� ���� �� /c
			const wchar_t* pszC = asCmdLine;
			while (*pszC != L'/') pszC --;
			nNewLen = pszC - pwszStartCmdLine;
			_ASSERTE(nNewLen>0);
			wcsncpy(pszNewCmd, pwszStartCmdLine, nNewLen);
			pszNewCmd[nNewLen] = 0; // !!! wcsncpy �� ������ ����������� '\0'
			// �������� ������ ��������
			if (!gbAttachMode) // ���� ����� ��� ��� � ���.������ - �������
				wcscat(pszNewCmd, L" /ATTACH ");
			if (!gbAlwaysConfirmExit) // ���� ����� ��� ��� � ���.������ - �������
				wcscat(pszNewCmd, L" /CONFIRM ");
			if (pszAddNewConArgs) {
				wcscat(pszNewCmd, L" ");
				wcscat(pszNewCmd, pszAddNewConArgs);
			}
			// ������� ������ ���� ������-��
			//2009-08-13 ���� �������������� (� ������ ComSpec ��������� /BW /BH /BZ �����������, �.�. ������ ���� �� FAR)
			//			 ������ ����������, ��� ��������� ������ (�� ������������� �� GUI) 
			//			 �� ������� ������������ � � ��������� ������� ��������
			//			 ���� ������� (������� ������ � GUI) � ReadConsoleData
			if (MyGetConsoleScreenBufferInfo(ghConOut, &cmd.sbi)) {
				int nBW = cmd.sbi.dwSize.X;
				int nBH = cmd.sbi.srWindow.Bottom - cmd.sbi.srWindow.Top + 1;
				int nBZ = cmd.sbi.dwSize.Y;
				if (nBZ <= nBH) nBZ = 0;
				wsprintf(pszNewCmd+wcslen(pszNewCmd), L" /BW=%i /BH=%i /BZ=%i ", nBW, nBH, nBZ);
			}
			//wcscat(pszNewCmd, L" </BW=9999 /BH=9999 /BZ=9999> ");
			// ������������ ����� �������
			// "cmd" ������ ��� ���� �� ������� �������� ������� � ������, ������� �� �� �����
			// cmd /c ""c:\program files\arc\7z.exe" -?"   // �� ��� � ������ ����� ���� ��������...
			// cmd /c "dir c:\"
			// � ��.
			// ���������� ���������� ������������� cmd
			if (lbIsNeedCmd) {
				CheckUnicodeMode();
				if (gnCmdUnicodeMode == 2)
					wcscat(pszNewCmd, L" /ROOT cmd /U /C ");
				else if (gnCmdUnicodeMode == 1)
					wcscat(pszNewCmd, L" /ROOT cmd /A /C ");
				else
					wcscat(pszNewCmd, L" /ROOT cmd /C ");
			} else {
				wcscat(pszNewCmd, L" /ROOT ");
			}
			// ������ �� ����������� ������� "-new_console"
			nNewLen = pwszCopy - asCmdLine;
			psFilePart = pszNewCmd + lstrlenW(pszNewCmd);
			wcsncpy(psFilePart, asCmdLine, nNewLen);
			psFilePart[nNewLen] = 0; // !!! wcsncpy �� ������ ����������� '\0'
			psFilePart += nNewLen;
			pwszCopy += nArgLen;
			// �������� � ������� ������� ���������� ��������� � �����������
			if (*pwszCopy) wcscpy(psFilePart, pwszCopy);
			//MessageBox(NULL, pszNewCmd, L"CmdLine", 0);
			//return 200;
			// ����� �����������
			*psNewCmd = pszNewCmd;
			// 26.06.2009 Maks - ����� ����� ����� - ��� ��������� ����� � ����� �������.
			DisableAutoConfirmExit();
			//srv.nProcessStartTick = GetTickCount() - 2*CHECK_ROOTSTART_TIMEOUT; //2010-03-06
			return 0;
		}

		//pwszCopy = asCmdLine;
		//if ((iRc = NextArg(&pwszCopy, szArg)) != 0) {
		//    wprintf (L"Parsing command line failed:\n%s\n", asCmdLine);
		//    return iRc;
		//}
		//pwszCopy = wcsrchr(szArg, L'\\'); if (!pwszCopy) pwszCopy = szArg;
	
		//#pragma warning( push )
		//#pragma warning(disable : 6400)
		//if (lstrcmpiW(pwszCopy, L"cmd")==0 || lstrcmpiW(pwszCopy, L"cmd.exe")==0) {
		//    bViaCmdExe = FALSE; // ��� ������ ��������� ���������, cmd.exe � ������ ��������� �� �����
		//}
		//#pragma warning( pop )
	//} else {
	//    bViaCmdExe = FALSE; // ��������� ����������� ��������� ��� ConEmuC (��������� �����)
	}

	if (gnRunMode == RM_COMSPEC && (!asCmdLine || !*asCmdLine)) {
		// � ���� ����� �������� ������ ���������� �� �����
		// *.ini -> "@" - ����� ��� ��� �� ������ �� ������ ��� ������� ����� �����, �� ComSpec �����...
		cmd.bNonGuiMode = TRUE;
		DisableAutoConfirmExit();
		return CERR_EMPTY_COMSPEC_CMDLINE;
	}

	bViaCmdExe = IsNeedCmd(asCmdLine, &lbNeedCutStartEndQuot);
	
	nCmdLine = lstrlenW(asCmdLine);

	if (!bViaCmdExe) {
		nCmdLine += 1; // ������ ����� ��� 0
	} else {
		// ���� ���������� ComSpecC - ������ ConEmuC ������������� ����������� ComSpec
		if (!GetEnvironmentVariable(L"ComSpecC", szComSpec, MAX_PATH) || szComSpec[0] == 0)
			if (!GetEnvironmentVariable(L"ComSpec", szComSpec, MAX_PATH) || szComSpec[0] == 0)
				szComSpec[0] = 0;
		if (szComSpec[0] != 0) {
			// ������ ���� ��� (��������) �� conemuc.exe
			//pwszCopy = wcsrchr(szComSpec, L'\\'); if (!pwszCopy) pwszCopy = szComSpec;
			pwszCopy = PointToName(szComSpec);
			#pragma warning( push )
			#pragma warning(disable : 6400)
			if (lstrcmpiW(pwszCopy, L"ConEmuC")==0 || lstrcmpiW(pwszCopy, L"ConEmuC.exe")==0)
				szComSpec[0] = 0;
			#pragma warning( pop )
		}
		
		// ComSpec/ComSpecC �� ���������, ���������� cmd.exe
		if (szComSpec[0] == 0) {
			if (!SearchPathW(NULL, L"cmd.exe", NULL, MAX_PATH, szComSpec, &psFilePart))
			{
				_printf ("Can't find cmd.exe!\n");
				return CERR_CMDEXENOTFOUND;
			}
		}

		nCmdLine += lstrlenW(szComSpec)+15; // "/C", ������� � ��������� "/U"
	}

	*psNewCmd = new wchar_t[nCmdLine];
	if (!(*psNewCmd))
	{
		_printf ("Can't allocate %i wchars!\n", (DWORD)nCmdLine);
		return CERR_NOTENOUGHMEM1;
	}
	
	// ��� ����� ��� ����� ��������� �������. ��� ������������� COMSPEC ������ ����, ����� �����
	lstrcpyW( *psNewCmd, asCmdLine );

	
	// ������ ��������� �������
	if (*asCmdLine == L'"') {
		if (asCmdLine[1]) {
			wchar_t *pszTitle = *psNewCmd;
			wchar_t *pszEndQ = pszTitle + lstrlenW(pszTitle) - 1;
			if (pszEndQ > (pszTitle+1) && *pszEndQ == L'"'
				&& wcschr(pszTitle+1, L'"') == pszEndQ)
			{
				*pszEndQ = 0; pszTitle ++;
				// "C:\Program Files\FAR\far.exe" - ������� �����, ����� �� ����������
				DWORD dwFileAttr = GetFileAttributes(pszTitle);
				if (dwFileAttr != INVALID_FILE_ATTRIBUTES && !(dwFileAttr & FILE_ATTRIBUTE_DIRECTORY))
					lbNeedCutStartEndQuot = FALSE;
				else
					lbNeedCutStartEndQuot = TRUE;
			} else {
				pszEndQ = NULL;
			}
			int nLen = 4096; //GetWindowTextLength(ghConWnd); -- KIS2009 ������ "������� �������� ���������"...
			if (nLen > 0) {
				gpszPrevConTitle = (wchar_t*)Alloc(nLen+1,2);
				if (gpszPrevConTitle) {
					if (!GetConsoleTitleW(gpszPrevConTitle, nLen+1)) {
						Free(gpszPrevConTitle); gpszPrevConTitle = NULL;
					}
				}
			}
			SetConsoleTitleW(pszTitle);
			if (pszEndQ) *pszEndQ = L'"';
		}
	} else if (*asCmdLine) {
		int nLen = 4096; //GetWindowTextLength(ghConWnd); -- KIS2009 ������ "������� �������� ���������"...
		if (nLen > 0) {
			gpszPrevConTitle = (wchar_t*)Alloc(nLen+1,2);
			if (gpszPrevConTitle) {
				if (!GetConsoleTitleW(gpszPrevConTitle, nLen+1)) {
					Free(gpszPrevConTitle); gpszPrevConTitle = NULL;
				}
			}
		}
		SetConsoleTitleW(asCmdLine);
	}
	
	if (bViaCmdExe)
	{
		CheckUnicodeMode();
		if (wcschr(szComSpec, L' ')) {
			(*psNewCmd)[0] = L'"';
			lstrcpyW( (*psNewCmd)+1, szComSpec );
			if (gnCmdUnicodeMode)
				lstrcatW( (*psNewCmd), (gnCmdUnicodeMode == 2) ? L" /U" : L" /A");
			lstrcatW( (*psNewCmd), cmd.bK ? L"\" /K " : L"\" /C " );
		} else {
			lstrcpyW( (*psNewCmd), szComSpec );
			if (gnCmdUnicodeMode)
				lstrcatW( (*psNewCmd), (gnCmdUnicodeMode == 2) ? L" /U" : L" /A");
			lstrcatW( (*psNewCmd), cmd.bK ? L" /K " : L" /C " );
		}
		// �������� ����� ���������� �� ���, � �� �������� ��������������
		//BOOL lbNeedQuatete = FALSE;
		// ������� � cmd.exe ����� ���������� ���:
		// ""c:\program files\arc\7z.exe" -?"
		//int nLastChar = lstrlenW(asCmdLine) - 1;
		//if (asCmdLine[0] == L'"' && asCmdLine[nLastChar] == L'"') {
		//	// �������� ����� ���������� �� ���, � �� �������� ��������������
		//	if (gnRunMode == RM_COMSPEC)
		//		lbNeedQuatete = FALSE;
		//	//if (asCmdLine[1] == L'"' && asCmdLine[2])
		//	//	lbNeedQuatete = FALSE; // ���
		//	//else if (wcschr(asCmdLine+1, L'"') == (asCmdLine+nLastChar))
		//	//	lbNeedQuatete = FALSE; // �� ���������. ������ ������� ���
		//} 
		//if (lbNeedQuatete) { // ����
		//	lstrcatW( (*psNewCmd), L"\"" );
		//}
		// ����������, ��������� ������
		lstrcatW( (*psNewCmd), asCmdLine );
		//if (lbNeedQuatete)
		//	lstrcatW( (*psNewCmd), L"\"" );
	} else if (lbNeedCutStartEndQuot) {
		// ""c:\arc\7z.exe -?"" - �� ����������!
		lstrcpyW( *psNewCmd, asCmdLine+1 );
		wchar_t *pszEndQ = *psNewCmd + lstrlenW(*psNewCmd) - 1;
		_ASSERTE(pszEndQ && *pszEndQ == L'"');
		if (pszEndQ && *pszEndQ == L'"') *pszEndQ = 0;
	}

#ifdef _DEBUG
	OutputDebugString(*psNewCmd); OutputDebugString(L"\n");
#endif
	
	return 0;
}

//int NextArg(LPCWSTR &asCmdLine, wchar_t* rsArg/*[MAX_PATH+1]*/)
//{
//    LPCWSTR psCmdLine = asCmdLine, pch = NULL;
//    wchar_t ch = *psCmdLine;
//    int nArgLen = 0;
//    
//    while (ch == L' ' || ch == L'\t' || ch == L'\r' || ch == L'\n') ch = *(++psCmdLine);
//    if (ch == 0) return CERR_CMDLINEEMPTY;
//
//    // �������� ���������� � "
//    if (ch == L'"') {
//        psCmdLine++;
//        pch = wcschr(psCmdLine, L'"');
//        if (!pch) return CERR_CMDLINE;
//        while (pch[1] == L'"') {
//            pch += 2;
//            pch = wcschr(pch, L'"');
//            if (!pch) return CERR_CMDLINE;
//        }
//        // ������ � pch ������ �� ��������� "
//    } else {
//        // �� ����� ������ ��� �� ������� �������
//        //pch = wcschr(psCmdLine, L' ');
//        // 09.06.2009 Maks - ��������� ��: cmd /c" echo Y "
//        pch = psCmdLine;
//        while (*pch && *pch!=L' ' && *pch!=L'"') pch++;
//        //if (!pch) pch = psCmdLine + wcslen(psCmdLine); // �� ����� ������
//    }
//    
//    nArgLen = pch - psCmdLine;
//    if (nArgLen > MAX_PATH) return CERR_CMDLINE;
//
//    // ������� ��������
//    memcpy(rsArg, psCmdLine, nArgLen*sizeof(wchar_t));
//    rsArg[nArgLen] = 0;
//
//    psCmdLine = pch;
//    
//    // Finalize
//    ch = *psCmdLine; // ����� ��������� �� ����������� �������
//    if (ch == L'"') ch = *(++psCmdLine);
//    while (ch == L' ' || ch == L'\t' || ch == L'\r' || ch == L'\n') ch = *(++psCmdLine);
//    asCmdLine = psCmdLine;
//    
//    return 0;
//}

void EmergencyShow()
{
	if (!IsWindowVisible(ghConWnd)) {
		SetWindowPos(ghConWnd, HWND_TOP, 50,50,0,0, SWP_NOSIZE);
		ShowWindowAsync(ghConWnd, SW_SHOWNORMAL);
	}
	if (!IsWindowEnabled(ghConWnd))
		EnableWindow(ghConWnd, true);
}

void ExitWaitForKey(WORD vkKey, LPCWSTR asConfirm, BOOL abNewLine, BOOL abDontShowConsole)
{
	// ����� ������ ���� ��������� �����
	if (!abDontShowConsole)
	{
		BOOL lbNeedVisible = FALSE;
		if (!ghConWnd) ghConWnd = GetConsoleWindow();
		if (ghConWnd) { // ���� ������� ���� ������
			WARNING("���� GUI ��� - �������� �� ������� SendMessageTimeout - ���������� ������� �� �����. �� ������� ����������");
			if (!IsWindowVisible(ghConWnd)) {
				BOOL lbGuiAlive = FALSE;
				if (ghConEmuWnd && IsWindow(ghConEmuWnd)) {
					DWORD_PTR dwLRc = 0;
					if (SendMessageTimeout(ghConEmuWnd, WM_NULL, 0, 0, SMTO_ABORTIFHUNG, 1000, &dwLRc))
						lbGuiAlive = TRUE;
				}
				if (!lbGuiAlive && !IsWindowVisible(ghConWnd)) {
					lbNeedVisible = TRUE;
					// �� ���� ��������... // ��������� "�����������" 80x25, ��� ��, ��� ���� �������� � ���.������
					//SMALL_RECT rcNil = {0}; SetConsoleSize(0, gcrBufferSize, rcNil, ":Exiting");
					//SetConsoleFontSizeTo(ghConWnd, 8, 12); // ��������� ����� ��������
					//ShowWindow(ghConWnd, SW_SHOWNORMAL); // � ������� ������
					EmergencyShow();
				}
			}
		}
	}

	// ������� ��������� �����
	INPUT_RECORD r = {0}; DWORD dwCount = 0;
	FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));

	PRINT_COMSPEC(L"Finalizing. gbInShutdown=%i\n", gbInShutdown);
	if (gbInShutdown)
		return; // Event �������� ��� �������������
	//
	_wprintf(asConfirm);

	//if (lbNeedVisible)
	// ���� ������ ����� ���� ������ - ������ GUI ����� ����, � ����� ������������ �� �����
	//while (PeekConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &r, 1, &dwCount)) {
	//    if (dwCount)
	//        ReadConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &r, 1, &dwCount);
	//    else
	//        Sleep(100);
	//    if (lbNeedVisible && !IsWindowVisible(ghConWnd)) {
	//        ShowWindow(ghConWnd, SW_SHOWNORMAL); // � ������� ������
	//    }
	while (ReadConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &r, 1, &dwCount)) {
		if (gnRunMode == RM_SERVER) {
			int nCount = srv.nProcessCount;
			if (nCount > 1) {
				// ! ������� ���� ����������, ����������� �� �����. ������� ������� � �����!
				WriteConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &r, 1, &dwCount);
				break;
			}
		}
	
		if (gbInShutdown ||
				(r.EventType == KEY_EVENT && r.Event.KeyEvent.bKeyDown 
				 && r.Event.KeyEvent.wVirtualKeyCode == vkKey))
			break;
	}
	//MessageBox(0,L"Debug message...............1",L"ConEmuC",0);
	//int nCh = _getch();
	if (abNewLine)
		_printf("\n");
}





void SendStarted()
{    
	static bool bSent = false;
	if (bSent)
		return; // �������� ������ ���� ���
	
	//crNewSize = cmd.sbi.dwSize;
	//_ASSERTE(crNewSize.X>=MIN_CON_WIDTH && crNewSize.Y>=MIN_CON_HEIGHT);

	HWND hConWnd = GetConsoleWindow();
	_ASSERTE(hConWnd == ghConWnd);
	ghConWnd = hConWnd;
	// ��� ComSpec-� ����� ����� ���������, � ����-�� ������ � ���� �������...
	if (gnRunMode == RM_COMSPEC) {
		DWORD nServerPID = 0, nGuiPID = 0;

		WCHAR sHeaderMapName[64];
		wsprintf(sHeaderMapName, CECONMAPNAME, (DWORD)hConWnd);
		HANDLE hFileMapping = OpenFileMapping(FILE_MAP_READ/*|FILE_MAP_WRITE*/, FALSE, sHeaderMapName);
		if (hFileMapping) {
			const CESERVER_REQ_CONINFO_HDR* pConsoleInfo 
				= (CESERVER_REQ_CONINFO_HDR*)MapViewOfFile(hFileMapping, FILE_MAP_READ/*|FILE_MAP_WRITE*/,0,0,0);
			if (pConsoleInfo) {
				nServerPID = pConsoleInfo->nServerPID;
				nGuiPID = pConsoleInfo->nGuiPID;
				if (pConsoleInfo->cbSize >= sizeof(CESERVER_REQ_CONINFO_HDR)) {
					if (pConsoleInfo->nLogLevel)
						CreateLogSizeFile(pConsoleInfo->nLogLevel);
				}
				UnmapViewOfFile(pConsoleInfo);
			}
			CloseHandle(hFileMapping);
		}

		if (nServerPID == 0) {
			cmd.bNonGuiMode = TRUE; // �� �������� ExecuteGuiCmd ��� ������. ��� �� ���� �������
			return; // ����� ComSpec, �� ������� ���, ��������������, � GUI ������ �������� �� �����
		}
	}
	
	CESERVER_REQ *pIn = NULL, *pOut = NULL;
	int nSize = sizeof(CESERVER_REQ_HDR)+sizeof(CESERVER_REQ_STARTSTOP);
	pIn = ExecuteNewCmd(CECMD_CMDSTARTSTOP, nSize);
	if (pIn) {
		pIn->StartStop.nStarted = (gnRunMode == RM_COMSPEC) ? 2 : 0; // Cmd/Srv ����� �����
		pIn->StartStop.hWnd = ghConWnd;
		pIn->StartStop.dwPID = gnSelfPID;
		//pIn->StartStop.dwInputTID = (gnRunMode == RM_SERVER) ? srv.dwInputThreadId : 0;
		if (gnRunMode == RM_SERVER)
			pIn->StartStop.bUserIsAdmin = IsUserAdmin();

		// ����� �������� 16��� ���������� ����� ������������ �������...
		gnImageSubsystem = 0;
		LPCWSTR pszTemp = gpszRunCmd;
		wchar_t lsRoot[MAX_PATH+1] = {0};
		if (gnRunMode == RM_SERVER && srv.bDebuggerActive) {
			// "��������"
			gnImageSubsystem = 0x101;
			gbRootIsCmdExe = TRUE; // ����� ����� ��������
		} else
		if (!gpszRunCmd) {
			// ����� �� ���-�������
			gnImageSubsystem = 0x100;
		} else
		if (0 == NextArg(&pszTemp, lsRoot)) {
			PRINT_COMSPEC(L"Starting: <%s>", lsRoot);
			if (!GetImageSubsystem(lsRoot, gnImageSubsystem))
				gnImageSubsystem = 0;
	   		PRINT_COMSPEC(L", Subsystem: <%i>\n", gnImageSubsystem);
			PRINT_COMSPEC(L"  Args: %s\n", pszTemp);
		}
		pIn->StartStop.nSubSystem = gnImageSubsystem;
		pIn->StartStop.bRootIsCmdExe = gbRootIsCmdExe; //2009-09-14
		// �� MyGet..., � �� ����� ���������������...
		HANDLE hOut = NULL;
		if (gnRunMode == RM_SERVER)
			hOut = (HANDLE)ghConOut;
		else
			hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		DWORD dwErr1 = 0;
		BOOL lbRc1 = GetConsoleScreenBufferInfo(hOut, &pIn->StartStop.sbi);
		if (!lbRc1) dwErr1 = GetLastError();

		PRINT_COMSPEC(L"Starting %s mode (ExecuteGuiCmd started)\n",(RunMode==RM_SERVER) ? L"Server" : L"ComSpec");
		
		pOut = ExecuteGuiCmd(ghConWnd, pIn, ghConWnd);
		if (!pOut) {
			// ��� ������ ������� GUI ����� �� ������ ������� ��������� �����, �.�.
			// �� ����� �������� �� ����������� ����������� ����, � ��� ������� GUI �� �����
			// ������� ����� ����-���� ���������, ���� GUI ������� ������� 
			// (event == EVENT_CONSOLE_START_APPLICATION && idObject == (LONG)mn_ConEmuC_PID)
			DWORD dwStart = GetTickCount(), dwDelta = 0;
			while (!gbInShutdown && dwDelta < GUIREADY_TIMEOUT) {
				Sleep(10);
				pOut = ExecuteGuiCmd(ghConWnd, pIn, ghConWnd);
				if (pOut) break;

				dwDelta = GetTickCount() - dwStart;
			}
			if (!pOut) {
				// �������� ��� ����������, ��� ������ ����� GUI ��������
				_ASSERTE(pOut != NULL);
			}
		}

		PRINT_COMSPEC(L"Starting %s mode (ExecuteGuiCmd finished)\n",(RunMode==RM_SERVER) ? L"Server" : L"ComSpec");

		if (pOut) {
			bSent = true;

			BOOL  bAlreadyBufferHeight = pOut->StartStopRet.bWasBufferHeight;

			DWORD nGuiPID = pOut->StartStopRet.dwPID;
			ghConEmuWnd = pOut->StartStopRet.hWnd;

			if (gnRunMode == RM_COMSPEC) {
				cmd.dwSrvPID = pOut->StartStopRet.dwSrvPID;
			}

			AllowSetForegroundWindow(nGuiPID);
			
			gnBufferHeight  = (SHORT)pOut->StartStopRet.nBufferHeight;
			gcrBufferSize.X = (SHORT)pOut->StartStopRet.nWidth;
			gcrBufferSize.Y = (SHORT)pOut->StartStopRet.nHeight;
			gbParmBufferSize = TRUE;
			
			if (gnRunMode == RM_SERVER) {
				if (srv.bDebuggerActive && !gnBufferHeight) gnBufferHeight = 1000;
				SMALL_RECT rcNil = {0};
				SetConsoleSize(gnBufferHeight, gcrBufferSize, rcNil, "::SendStarted");
				// ����� ��������� ����������
				if (pOut->StartStopRet.bNeedLangChange) {
					#ifndef INPUTLANGCHANGE_SYSCHARSET
					#define INPUTLANGCHANGE_SYSCHARSET 0x0001
					#endif
					WPARAM wParam = INPUTLANGCHANGE_SYSCHARSET;
					TODO("��������� �� x64, �� ����� �� ������� � 0xFFFFFFFFFFFFFFFFFFFFF");
					LPARAM lParam = (LPARAM)(DWORD_PTR)pOut->StartStopRet.NewConsoleLang;
					SendMessage(ghConWnd, WM_INPUTLANGCHANGEREQUEST, wParam, lParam);
				}
			} else
			// ����� ��� ����������, ��� ���� COMSPEC ������� �� �������.
			if (bAlreadyBufferHeight)
				cmd.bNonGuiMode = TRUE; // �� �������� ExecuteGuiCmd ��� ������ - ��������� ������ ��������

			//nNewBufferHeight = ((DWORD*)(pOut->Data))[0];
			//crNewSize.X = (SHORT)((DWORD*)(pOut->Data))[1];
			//crNewSize.Y = (SHORT)((DWORD*)(pOut->Data))[2];
			TODO("���� �� ������� ��� COMSPEC - �� � GUI �������� ��������� ����� �� ������");
			//if (rNewWindow.Right >= crNewSize.X) // ������ ��� �������� �� ���� ������ ���������
			//    rNewWindow.Right = crNewSize.X-1;
			ExecuteFreeResult(pOut); pOut = NULL;

			//gnBufferHeight = nNewBufferHeight;
		} else {
			cmd.bNonGuiMode = TRUE; // �� �������� ExecuteGuiCmd ��� ������. ��� �� ���� �������
		}
		ExecuteFreeResult(pIn); pIn = NULL;
	}
}


WARNING("�������� LogInput(INPUT_RECORD* pRec) �� ��� ����� ������� 'ConEmuC-input-%i.log'");
void CreateLogSizeFile(int nLevel)
{
	if (ghLogSize) return; // ���
	
	DWORD dwErr = 0;
	wchar_t szFile[MAX_PATH+64], *pszDot;
	
	if (!GetModuleFileName(NULL, szFile, MAX_PATH)) {
		dwErr = GetLastError();
		_printf("GetModuleFileName failed! ErrCode=0x%08X\n", dwErr);
		return; // �� �������
	}
	if ((pszDot = wcsrchr(szFile, L'.')) == NULL) {
		_printf("wcsrchr failed!\n", 0, szFile);
		return; // ������
	}
	wsprintfW(pszDot, L"-size-%i.log", gnSelfPID);
	
	ghLogSize = CreateFileW ( szFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (ghLogSize == INVALID_HANDLE_VALUE) {
		ghLogSize = NULL;
		dwErr = GetLastError();
		_printf("Create console log file failed! ErrCode=0x%08X\n", dwErr, szFile);
		return;
	}
	
	int nLen = lstrlen(szFile);
	wpszLogSizeFile = /*_wcsdup(szFile);*/(wchar_t*)calloc(nLen+1,2);
	lstrcpy(wpszLogSizeFile, szFile);
	// OK, ��� �������
	LPCSTR pszCmdLine = GetCommandLineA();
	if (pszCmdLine) {
		WriteFile(ghLogSize, pszCmdLine, (DWORD)strlen(pszCmdLine), &dwErr, 0);
		WriteFile(ghLogSize, "\r\n", 2, &dwErr, 0);
	}
	LogSize(NULL, "Startup");
}

void LogString(LPCSTR asText)
{
	if (!ghLogSize) return;

	char szInfo[255]; szInfo[0] = 0;
	LPCSTR pszThread = "<unknown thread>";

	DWORD dwId = GetCurrentThreadId();
	if (dwId == gdwMainThreadId)
		pszThread = "MainThread";
	else
	if (dwId == srv.dwServerThreadId)
		pszThread = "ServerThread";
	else
	if (dwId == srv.dwRefreshThread)
		pszThread = "RefreshThread";
	else
	if (dwId == srv.dwWinEventThread)
		pszThread = "WinEventThread";
	else
	//if (dwId == srv.dwInputThreadId)
	//	pszThread = "InputThread";
	//else
	if (dwId == srv.dwInputPipeThreadId)
		pszThread = "InputPipeThread";


	SYSTEMTIME st; GetLocalTime(&st);
	wsprintfA(szInfo, "%i:%02i:%02i ",
		st.wHour, st.wMinute, st.wSecond);
	int nCur = lstrlenA(szInfo);
	lstrcpynA(szInfo+nCur, asText ? asText : "", 255-nCur-3);
	lstrcatA(szInfo, "\r\n");


	DWORD dwLen = 0;
	WriteFile(ghLogSize, szInfo, (DWORD)strlen(szInfo), &dwLen, 0);
	FlushFileBuffers(ghLogSize);
}

void LogSize(COORD* pcrSize, LPCSTR pszLabel)
{
	if (!ghLogSize) return;
	
	CONSOLE_SCREEN_BUFFER_INFO lsbi = {{0,0}};
	// � �������� ��� �������� �������� ��������
	GetConsoleScreenBufferInfo(ghConOut ? ghConOut : GetStdHandle(STD_OUTPUT_HANDLE), &lsbi);
	
	char szInfo[192]; szInfo[0] = 0;
	LPCSTR pszThread = "<unknown thread>";
	
	DWORD dwId = GetCurrentThreadId();
	if (dwId == gdwMainThreadId)
			pszThread = "MainThread";
			else
	if (dwId == srv.dwServerThreadId)
			pszThread = "ServerThread";
			else
	if (dwId == srv.dwRefreshThread)
			pszThread = "RefreshThread";
			else
	if (dwId == srv.dwWinEventThread)
			pszThread = "WinEventThread";
			else
	//if (dwId == srv.dwInputThreadId)
	//		pszThread = "InputThread";
	//		else
	if (dwId == srv.dwInputPipeThreadId)
			pszThread = "InputPipeThread";
			
	/*HDESK hDesk = GetThreadDesktop ( GetCurrentThreadId() );
	HDESK hInp = OpenInputDesktop ( 0, FALSE, GENERIC_READ );*/
	
			
	SYSTEMTIME st; GetLocalTime(&st);
	char szMapSize[32]; szMapSize[0] = 0;
	if (srv.pConsoleInfo) {
		wsprintfA(szMapSize, " CurMapSize={%ix%ix%i}",
			srv.pConsoleInfo->sbi.dwSize.X, srv.pConsoleInfo->sbi.dwSize.Y,
			srv.pConsoleInfo->sbi.srWindow.Bottom-srv.pConsoleInfo->sbi.srWindow.Top+1);
	}
	
	if (pcrSize) {
		wsprintfA(szInfo, "%i:%02i:%02i.%03i CurSize={%ix%i}%s ChangeTo={%ix%i} %s %s\r\n",
			st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
			lsbi.dwSize.X, lsbi.dwSize.Y, szMapSize, pcrSize->X, pcrSize->Y, pszThread, (pszLabel ? pszLabel : ""));
	} else {
		wsprintfA(szInfo, "%i:%02i:%02i.%03i CurSize={%ix%i}%s %s %s\r\n",
			st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
			lsbi.dwSize.X, lsbi.dwSize.Y, szMapSize, pszThread, (pszLabel ? pszLabel : ""));
	}
	
	//if (hInp) CloseDesktop ( hInp );
	
	DWORD dwLen = 0;
	WriteFile(ghLogSize, szInfo, (DWORD)strlen(szInfo), &dwLen, 0);
	FlushFileBuffers(ghLogSize);
}



BOOL CheckProcessCount(BOOL abForce/*=FALSE*/)
{
	static DWORD dwLastCheckTick = GetTickCount();

	UINT nPrevCount = srv.nProcessCount;
	if (srv.nProcessCount <= 0) {
		abForce = TRUE;
	}

	if (!abForce) {
		DWORD dwCurTick = GetTickCount();
		if ((dwCurTick - dwLastCheckTick) < (DWORD)CHECK_PROCESSES_TIMEOUT)
			return FALSE;
	}

	BOOL lbChanged = FALSE;
	MSectionLock CS; CS.Lock(srv.csProc);

	if (srv.nProcessCount == 0) {
		srv.pnProcesses[0] = gnSelfPID;
		srv.nProcessCount = 1;
	}

	if (srv.bDebuggerActive) {
		//if (srv.hRootProcess) {
		//	if (WaitForSingleObject(srv.hRootProcess, 0) == WAIT_OBJECT_0) {
		//		srv.nProcessCount = 1;
		//		return TRUE;
		//	}
		//}
		//srv.pnProcesses[1] = srv.dwRootProcess;
		//srv.nProcessCount = 2;
		return FALSE;
	}

	if (!pfnGetConsoleProcessList) {

		if (srv.hRootProcess) {
			if (WaitForSingleObject(srv.hRootProcess, 0) == WAIT_OBJECT_0) {
				srv.pnProcesses[1] = 0;
				lbChanged = srv.nProcessCount != 1;
				srv.nProcessCount = 1;
			} else {
				srv.pnProcesses[1] = srv.dwRootProcess;
				lbChanged = srv.nProcessCount != 2;
				srv.nProcessCount = 2;
			}
		}

	} else {
		DWORD nCurCount = 0;

		nCurCount = pfnGetConsoleProcessList(srv.pnProcesses, srv.nMaxProcesses);
		lbChanged = srv.nProcessCount != nCurCount;

		if (nCurCount == 0) {
			// ��� ������ � Win7 �������� conhost.exe
			#ifdef _DEBUG
			DWORD dwErr = GetLastError();
			#endif
			srv.nProcessCount = 1;
			SetEvent(ghQuitEvent);
			SetEvent(ghExitQueryEvent);
			return TRUE;
		}

		if (nCurCount > srv.nMaxProcesses) {
			DWORD nSize = nCurCount + 100;
			DWORD* pnPID = (DWORD*)Alloc(nSize, sizeof(DWORD));
			if (pnPID) {
				
				CS.RelockExclusive(200);

				nCurCount = pfnGetConsoleProcessList(pnPID, nSize);
				if (nCurCount > 0 && nCurCount <= nSize) {
					Free(srv.pnProcesses);
					srv.pnProcesses = pnPID; pnPID = NULL;
					srv.nProcessCount = nCurCount;
					srv.nMaxProcesses = nSize;
				}

				if (pnPID)
					Free(pnPID);
			}
		} else {
			// �������� � 0 ������ �� ������� ����������
			_ASSERTE(srv.nProcessCount < srv.nMaxProcesses);
			if (nCurCount < srv.nProcessCount) {
				UINT nSize = sizeof(DWORD)*(srv.nProcessCount - nCurCount);
				memset(srv.pnProcesses + nCurCount, 0, nSize);
			}
			_ASSERTE(nCurCount>0);
			srv.nProcessCount = nCurCount;
		}

		if (!lbChanged) {
			UINT nSize = sizeof(DWORD)*min(srv.nMaxProcesses,START_MAX_PROCESSES);
			#ifdef _DEBUG
			_ASSERTE(!IsBadWritePtr(srv.pnProcessesCopy,nSize));
			_ASSERTE(!IsBadWritePtr(srv.pnProcesses,nSize));
			#endif
			lbChanged = memcmp(srv.pnProcessesCopy, srv.pnProcesses, nSize) != 0;
			MCHKHEAP;
			if (lbChanged)
				memmove(srv.pnProcessesCopy, srv.pnProcesses, nSize);
			MCHKHEAP;
		}
		
		if (lbChanged)
		{
			BOOL lbFarExists = FALSE, lbTelnetExist = FALSE;
			if (srv.nProcessCount > 1)
			{
				HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
				if (hSnap != INVALID_HANDLE_VALUE) {
					PROCESSENTRY32 prc = {sizeof(PROCESSENTRY32)};
					if (Process32First(hSnap, &prc))
					{
						do
						{
							for (UINT i = 0; i < srv.nProcessCount; i++) {
								if (prc.th32ProcessID != gnSelfPID
									&& prc.th32ProcessID == srv.pnProcesses[i])
								{
									if (lstrcmpiW(prc.szExeFile, L"far.exe")==0) {
										lbFarExists = TRUE;
										//if (srv.nProcessCount <= 2) // ����� ��������� � ntvdm
										//	break; // ��������, � ������� ��� ���� � telnet?
									} else if (!srv.nNtvdmPID && lstrcmpiW(prc.szExeFile, L"ntvdm.exe")==0) {
										srv.nNtvdmPID = prc.th32ProcessID;
									}
									// �� ����� ������ Telnet ���� ����� ������ ��� �������!
									//2009-12-28 �����. ��� ������ ���� ����...
									//if (lstrcmpiW(prc.szExeFile, L"telnet.exe")==0) {
									//	// ����� ������ �����������
									//	ghConIn.Close(); ghConOut.Close();
									//	//srv.bWinHookAllow = TRUE; // ��������� ��������� ������� ��� �������
									//	lbFarExists = TRUE; lbTelnetExist = TRUE; break;
									//}
								}
							}
						} while (!(lbFarExists && lbTelnetExist) && Process32Next(hSnap, &prc));
					}
					CloseHandle(hSnap);
				}
			}
			srv.bTelnetActive = lbTelnetExist;
			
			//if (srv.nProcessCount >= 2
			//	&& ( (srv.hWinHook == NULL && srv.bWinHookAllow) || (srv.hWinHook != NULL) )
			//    )
			//{
			//	if (lbFarExists) srv.nWinHookMode = 2; else srv.nWinHookMode = 1;
			//	
			// 			if (lbFarExists && srv.hWinHook == NULL && srv.bWinHookAllow) {
			// 				HookWinEvents(2);
			// 			} else if (!lbFarExists && srv.hWinHook) {
			// 				HookWinEvents(0);
			// 			}
			//}
		}
	}

	dwLastCheckTick = GetTickCount();

	// ���� �������� ������� ���������� ���������� (10 ���), ������ �� ����� � gbAlwaysConfirmExit ����� ��������
	// ���� gbAutoDisableConfirmExit==FALSE - ����� ������������� �������� ������� �� �����������
	if (gbAlwaysConfirmExit // ���� ��� �� �������
		&& gbAutoDisableConfirmExit // ��������� �� ������ ����� �������� (10 ���)
		&& nPrevCount > 1 // ���� � ������� ��� ������������ ���������� �������
		&& srv.hRootProcess) // � �������� ������� ��� ������ �������
	{
		if ((dwLastCheckTick - srv.nProcessStartTick) > CHECK_ROOTOK_TIMEOUT) {
			// ��� �������� ����������� ���� ���
			gbAutoDisableConfirmExit = FALSE;

			// 10 ���. ������, ������ ���������� ���������, � ��� �� �������?
			DWORD dwProcWait = WaitForSingleObject(srv.hRootProcess, 0);
			if (dwProcWait == WAIT_OBJECT_0) {
				// �������� ������� ��������, ��������, ���� �����-�� ��������?
				gbRootAliveLess10sec = TRUE; // �������� ������� ���������� ����� 10 ���
			} else {
				// �������� ������� ��� ��� ��������, ������� ��� ��� �� � ������������� �������� ������� �� �����������
				DisableAutoConfirmExit();
				//srv.nProcessStartTick = GetTickCount() - 2*CHECK_ROOTSTART_TIMEOUT; // ������ nProcessStartTick �� �����. �������� ������ �� �������
			}
		}
	}
	if (gbRootWasFoundInCon == 0 && srv.nProcessCount > 1 && srv.hRootProcess && srv.dwRootProcess) {
		if (WaitForSingleObject(srv.hRootProcess, 0) == WAIT_OBJECT_0) {
			gbRootWasFoundInCon = 2; // � ������� ������� �� �����, � ��� ����������
		} else {
			for (UINT n = 0; n < srv.nProcessCount; n++) {
				if (srv.dwRootProcess == srv.pnProcesses[n]) {
					// ������� ����� � �������
					gbRootWasFoundInCon = 1; break;
				}
			}			
		}
	}

	// ��������� � ������� �� ��������?
	#ifndef WIN64
	if (srv.nProcessCount == 2 && !srv.bNtvdmActive && srv.nNtvdmPID) {
		// �������� ���� �������� 16������ ����������, � ntvdm.exe �� ����������� ��� ��� ��������
		// gnSelfPID �� ����������� ����� � srv.pnProcesses[0]
		if ((srv.pnProcesses[0] == gnSelfPID && srv.pnProcesses[1] == srv.nNtvdmPID)
			|| (srv.pnProcesses[1] == gnSelfPID && srv.pnProcesses[0] == srv.nNtvdmPID))
		{
			// ������� � ���� ������� ������� ��������
			PostMessage(ghConWnd, WM_CLOSE, 0, 0);
		}
	}
	#endif
	WARNING("���� � ������� �� ����� ���� �������� - ��� ������� ���� 'srv.nProcessCount == 1' ����������");
	// ������ - ����������� �� ����. ���������� ��������� ���������� - 5
	// cmd ������������ ����� (path not found)
	// ���������� ��������� �������� 5 � �� ���� �� ���� ������� �� ��������
	if (nPrevCount == 1 && srv.nProcessCount == 1 && srv.nProcessStartTick &&
		((dwLastCheckTick - srv.nProcessStartTick) > CHECK_ROOTSTART_TIMEOUT) &&
		WaitForSingleObject(ghExitQueryEvent,0) == WAIT_TIMEOUT)
	{
		nPrevCount = 2; // ����� ��������� ��������� �������
		//2010-03-06 - ��������� ������ ������ ���� ��� ������ �������
		//if (!gbAlwaysConfirmExit) gbAlwaysConfirmExit = TRUE; // ����� ������� �� �����������
	}
	if (nPrevCount > 1 && srv.nProcessCount == 1)
	{
		if (srv.pnProcesses[0] != gnSelfPID) {
			_ASSERTE(srv.pnProcesses[0] == gnSelfPID);
		} else {
			CS.Unlock();
			//2010-03-06 ��� �� �����, �������� �������� �� �������
			//if (!gbAlwaysConfirmExit && (dwLastCheckTick - srv.nProcessStartTick) <= CHECK_ROOTSTART_TIMEOUT) {
			//	gbAlwaysConfirmExit = TRUE; // ����� ������� �� �����������
			//}
			if (gbAlwaysConfirmExit && (dwLastCheckTick - srv.nProcessStartTick) <= CHECK_ROOTSTART_TIMEOUT)
				gbRootAliveLess10sec = TRUE; // �������� ������� ���������� ����� 10 ���
			SetEvent(ghExitQueryEvent);
		}
	}

	return lbChanged;
}

int CALLBACK FontEnumProc(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, DWORD FontType, LPARAM lParam)
{
	if ((FontType & TRUETYPE_FONTTYPE) == TRUETYPE_FONTTYPE) {
		// OK, ��������
		lstrcpyW(srv.szConsoleFont, lpelfe->elfLogFont.lfFaceName);
		return 0;
	}
	return TRUE; // ���� ��������� ����
}

void ProcessDebugEvent()
{
	static wchar_t wszDbgText[1024];
	static char szDbgText[1024];
	BOOL lbNonContinuable = FALSE;

	DEBUG_EVENT evt = {0};
	if (WaitForDebugEvent(&evt,10)) {
		lbNonContinuable = FALSE;
		
		switch (evt.dwDebugEventCode) {
		case CREATE_PROCESS_DEBUG_EVENT:
		case CREATE_THREAD_DEBUG_EVENT:
		case EXIT_PROCESS_DEBUG_EVENT:
		case EXIT_THREAD_DEBUG_EVENT:
		case RIP_EVENT:
		{
			LPCSTR pszName = "Unknown";
			switch (evt.dwDebugEventCode) {
			case CREATE_PROCESS_DEBUG_EVENT: pszName = "CREATE_PROCESS_DEBUG_EVENT"; break;
			case CREATE_THREAD_DEBUG_EVENT: pszName = "CREATE_THREAD_DEBUG_EVENT"; break;
			case EXIT_PROCESS_DEBUG_EVENT: pszName = "EXIT_PROCESS_DEBUG_EVENT"; break;
			case EXIT_THREAD_DEBUG_EVENT: pszName = "EXIT_THREAD_DEBUG_EVENT"; break;
			case RIP_EVENT: pszName = "RIP_EVENT"; break;
			}
			wsprintfA(szDbgText, "{%i.%i} %s\n", evt.dwProcessId,evt.dwThreadId, pszName);
			_printf(szDbgText);
			break;
		} 

		
		case LOAD_DLL_DEBUG_EVENT:
		{
			LPCSTR pszName = "Unknown";
			switch (evt.dwDebugEventCode) {
			case LOAD_DLL_DEBUG_EVENT: pszName = "LOAD_DLL_DEBUG_EVENT"; break;
				//6 Reports a load-dynamic-link-library (DLL) debugging event. The value of u.LoadDll specifies a LOAD_DLL_DEBUG_INFO structure.
			case UNLOAD_DLL_DEBUG_EVENT: pszName = "UNLOAD_DLL_DEBUG_EVENT"; break;
				//7 Reports an unload-DLL debugging event. The value of u.UnloadDll specifies an UNLOAD_DLL_DEBUG_INFO structure.
			}
			wsprintfA(szDbgText, "{%i.%i} %s\n", evt.dwProcessId,evt.dwThreadId, pszName);
			_printf(szDbgText);
			break;
		}


		case EXCEPTION_DEBUG_EVENT:
		//1 Reports an exception debugging event. The value of u.Exception specifies an EXCEPTION_DEBUG_INFO structure.
		{
			lbNonContinuable = (evt.u.Exception.ExceptionRecord.ExceptionFlags&EXCEPTION_NONCONTINUABLE)==EXCEPTION_NONCONTINUABLE;
			switch(evt.u.Exception.ExceptionRecord.ExceptionCode)
			{
			case EXCEPTION_ACCESS_VIOLATION: // The thread tried to read from or write to a virtual address for which it does not have the appropriate access.
			if (evt.u.Exception.ExceptionRecord.NumberParameters>=2) {
				wsprintfA(szDbgText,"{%i.%i} EXCEPTION_ACCESS_VIOLATION at 0x%08X flags 0x%08X%s %s of 0x%08X\n", evt.dwProcessId,evt.dwThreadId,
					evt.u.Exception.ExceptionRecord.ExceptionAddress,
					evt.u.Exception.ExceptionRecord.ExceptionFlags,
					((evt.u.Exception.ExceptionRecord.ExceptionFlags&EXCEPTION_NONCONTINUABLE) ? "(EXCEPTION_NONCONTINUABLE)" : ""),
					((evt.u.Exception.ExceptionRecord.ExceptionInformation[0]==0) ? "Read" :
					(evt.u.Exception.ExceptionRecord.ExceptionInformation[0]==1) ? "Write" :
					(evt.u.Exception.ExceptionRecord.ExceptionInformation[0]==8) ? "DEP" : "???"),
					evt.u.Exception.ExceptionRecord.ExceptionInformation[1]
					);
			} else {
				wsprintfA(szDbgText,"{%i.%i} EXCEPTION_ACCESS_VIOLATION at 0x%08X flags 0x%08X%s\n", evt.dwProcessId,evt.dwThreadId,
					evt.u.Exception.ExceptionRecord.ExceptionAddress,
					evt.u.Exception.ExceptionRecord.ExceptionFlags,
					(evt.u.Exception.ExceptionRecord.ExceptionFlags&EXCEPTION_NONCONTINUABLE) ? "(EXCEPTION_NONCONTINUABLE)" : "");
			}
			_printf(szDbgText);
			break; 

			default:
			{
				char szName[32]; LPCSTR pszName; pszName = szName;
				#define EXCASE(s) case s: pszName = #s; break
				switch (evt.u.Exception.ExceptionRecord.ExceptionCode) {
				EXCASE(EXCEPTION_ARRAY_BOUNDS_EXCEEDED); // The thread tried to access an array element that is out of bounds and the underlying hardware supports bounds checking.
				EXCASE(EXCEPTION_BREAKPOINT); // A breakpoint was encountered.
				EXCASE(EXCEPTION_DATATYPE_MISALIGNMENT); // The thread tried to read or write data that is misaligned on hardware that does not provide alignment. For example, 16-bit values must be aligned on 2-byte boundaries; 32-bit values on 4-byte boundaries, and so on.
				EXCASE(EXCEPTION_FLT_DENORMAL_OPERAND); // One of the operands in a floating-point operation is denormal. A denormal value is one that is too small to represent as a standard floating-point value.
				EXCASE(EXCEPTION_FLT_DIVIDE_BY_ZERO); // The thread tried to divide a floating-point value by a floating-point divisor of zero.
				EXCASE(EXCEPTION_FLT_INEXACT_RESULT); // The result of a floating-point operation cannot be represented exactly as a decimal fraction.
				EXCASE(EXCEPTION_FLT_INVALID_OPERATION); // This exception represents any floating-point exception not included in this list.
				EXCASE(EXCEPTION_FLT_OVERFLOW); // The exponent of a floating-point operation is greater than the magnitude allowed by the corresponding type.
				EXCASE(EXCEPTION_FLT_STACK_CHECK); // The stack overflowed or underflowed as the result of a floating-point operation.
				EXCASE(EXCEPTION_FLT_UNDERFLOW); // The exponent of a floating-point operation is less than the magnitude allowed by the corresponding type.
				EXCASE(EXCEPTION_ILLEGAL_INSTRUCTION); // The thread tried to execute an invalid instruction.
				EXCASE(EXCEPTION_IN_PAGE_ERROR); // The thread tried to access a page that was not present, and the system was unable to load the page. For example, this exception might occur if a network connection is lost while running a program over the network.
				EXCASE(EXCEPTION_INT_DIVIDE_BY_ZERO); // The thread tried to divide an integer value by an integer divisor of zero.
				EXCASE(EXCEPTION_INT_OVERFLOW); // The result of an integer operation caused a carry out of the most significant bit of the result.
				EXCASE(EXCEPTION_INVALID_DISPOSITION); // An exception handler returned an invalid disposition to the exception dispatcher. Programmers using a high-level language such as C should never encounter this exception.
				EXCASE(EXCEPTION_NONCONTINUABLE_EXCEPTION); // The thread tried to continue execution after a noncontinuable exception occurred.
				EXCASE(EXCEPTION_PRIV_INSTRUCTION); // The thread tried to execute an instruction whose operation is not allowed in the current machine mode.
				EXCASE(EXCEPTION_SINGLE_STEP); // A trace trap or other single-instruction mechanism signaled that one instruction has been executed.
				EXCASE(EXCEPTION_STACK_OVERFLOW); // The thread used up its stack.
				default:
					wsprintfA(szName,"Exception 0x%08X",evt.u.Exception.ExceptionRecord.ExceptionCode);
				}
				wsprintfA(szDbgText,"{%i.%i} Exception 0x%08X at 0x%08X flags 0x%08X%s\n", evt.dwProcessId,evt.dwThreadId,
					pszName,
					evt.u.Exception.ExceptionRecord.ExceptionAddress,
					evt.u.Exception.ExceptionRecord.ExceptionFlags,
					(evt.u.Exception.ExceptionRecord.ExceptionFlags&EXCEPTION_NONCONTINUABLE) ? "(EXCEPTION_NONCONTINUABLE)" : "");
				_printf(szDbgText);
			}
			}
		}
		break;
		
		
		case OUTPUT_DEBUG_STRING_EVENT:
		//8 Reports an output-debugging-string debugging event. The value of u.DebugString specifies an OUTPUT_DEBUG_STRING_INFO structure.
		{
			wszDbgText[0] = 0;
			if (evt.u.DebugString.nDebugStringLength >= 1024) evt.u.DebugString.nDebugStringLength = 1023;
			DWORD_PTR nRead = 0;
			if (evt.u.DebugString.fUnicode) {
				if (!ReadProcessMemory(srv.hRootProcess, evt.u.DebugString.lpDebugStringData, wszDbgText, 2*evt.u.DebugString.nDebugStringLength, &nRead))
					lstrcpy(wszDbgText, L"???");
				else
					wszDbgText[min(1023,nRead+1)] = 0;
			} else {
				if (!ReadProcessMemory(srv.hRootProcess, evt.u.DebugString.lpDebugStringData, szDbgText, evt.u.DebugString.nDebugStringLength, &nRead))
					lstrcpy(wszDbgText, L"???");
				else {
					szDbgText[min(1023,nRead+1)] = 0;
					MultiByteToWideChar(CP_ACP, 0, szDbgText, -1, wszDbgText, 1024);
				}
			}
			_printf("{%i.%i} ", evt.dwProcessId,evt.dwThreadId, wszDbgText);
		}
		break;
		
		}
		
		// ���������� ������������ �������
		ContinueDebugEvent(evt.dwProcessId, evt.dwThreadId, DBG_EXCEPTION_NOT_HANDLED);
	}
}




DWORD WINAPI ServerThread(LPVOID lpvParam) 
{ 
   BOOL fConnected = FALSE;
   DWORD dwInstanceThreadId = 0, dwErr = 0;
   HANDLE hPipe = NULL, hInstanceThread = NULL;
   
 
// The main loop creates an instance of the named pipe and 
// then waits for a client to connect to it. When the client 
// connects, a thread is created to handle communications 
// with that client, and the loop is repeated. 
 
   for (;;) 
   { 
	  MCHKHEAP;
	  hPipe = CreateNamedPipe( 
		  srv.szPipename,               // pipe name 
		  PIPE_ACCESS_DUPLEX,       // read/write access 
		  PIPE_TYPE_MESSAGE |       // message type pipe 
		  PIPE_READMODE_MESSAGE |   // message-read mode 
		  PIPE_WAIT,                // blocking mode 
		  PIPE_UNLIMITED_INSTANCES, // max. instances  
		  PIPEBUFSIZE,              // output buffer size 
		  PIPEBUFSIZE,              // input buffer size 
		  0,                        // client time-out 
		  gpNullSecurity);          // default security attribute 

	  _ASSERTE(hPipe != INVALID_HANDLE_VALUE);
	  
	  if (hPipe == INVALID_HANDLE_VALUE) 
	  {
		  dwErr = GetLastError();
		  _printf("CreateNamedPipe failed, ErrCode=0x%08X\n", dwErr); 
		  Sleep(10);
		  continue;
	  }
 
	  // Wait for the client to connect; if it succeeds, 
	  // the function returns a nonzero value. If the function
	  // returns zero, GetLastError returns ERROR_PIPE_CONNECTED. 
 
	  fConnected = ConnectNamedPipe(hPipe, NULL) ? 
		 TRUE : (GetLastError() == ERROR_PIPE_CONNECTED); 

	  if (WaitForSingleObject(ghQuitEvent, 0) == WAIT_OBJECT_0)
		  break;
 
	  MCHKHEAP;
	  if (fConnected) 
	  { 
	  // Create a thread for this client. 
		 hInstanceThread = CreateThread( 
			NULL,              // no security attribute 
			0,                 // default stack size 
			InstanceThread,    // thread proc
			(LPVOID) hPipe,    // thread parameter 
			0,                 // not suspended 
			&dwInstanceThreadId);      // returns thread ID 

		 if (hInstanceThread == NULL) 
		 {
			dwErr = GetLastError();
			_printf("CreateThread(Instance) failed, ErrCode=0x%08X\n", dwErr);
			Sleep(10);
			continue;
		 }
		 else {
			 SafeCloseHandle(hInstanceThread); 
		 }
	   } 
	  else {
		// The client could not connect, so close the pipe. 
		SafeCloseHandle(hPipe); 
	  }
	  MCHKHEAP;
   } 
   return 1; 
} 

DWORD WINAPI InstanceThread(LPVOID lpvParam) 
{ 
	CESERVER_REQ in={0}, *pIn=NULL, *pOut=NULL;
	DWORD cbBytesRead, cbWritten, dwErr = 0;
	BOOL fSuccess; 
	HANDLE hPipe; 

	// The thread's parameter is a handle to a pipe instance. 
	hPipe = (HANDLE) lpvParam; 

	MCHKHEAP;
 
	// Read client requests from the pipe. 
	memset(&in, 0, sizeof(in));
	fSuccess = ReadFile(
		hPipe,        // handle to pipe 
		&in,          // buffer to receive data 
		sizeof(in),   // size of buffer 
		&cbBytesRead, // number of bytes read 
		NULL);        // not overlapped I/O 

	if ((!fSuccess && ((dwErr = GetLastError()) != ERROR_MORE_DATA)) ||
			cbBytesRead < sizeof(CESERVER_REQ_HDR) || in.hdr.nSize < sizeof(CESERVER_REQ_HDR))
	{
		goto wrap;
	}

	if (in.hdr.nSize > cbBytesRead)
	{
		DWORD cbNextRead = 0;
		// ��� ������ Alloc, � �� ExecuteNewCmd, �.�. ������ ������ �������, � �� ����������� �����
		pIn = (CESERVER_REQ*)Alloc(in.hdr.nSize, 1);
		if (!pIn)
			goto wrap;
		memmove(pIn, &in, cbBytesRead); // ������ ��������� ����������

		fSuccess = ReadFile(
			hPipe,        // handle to pipe 
			((LPBYTE)pIn)+cbBytesRead,  // buffer to receive data 
			in.hdr.nSize - cbBytesRead,   // size of buffer 
			&cbNextRead, // number of bytes read 
			NULL);        // not overlapped I/O 
		if (fSuccess)
			cbBytesRead += cbNextRead;
	}

	if (!GetAnswerToRequest(pIn ? *pIn : in, &pOut) || pOut==NULL) {
		// ���� ���������� ��� - ��� ����� ���-������ �������, ����� TransactNamedPipe ����� �������?
		CESERVER_REQ_HDR Out={0};
		ExecutePrepareCmd((CESERVER_REQ*)&Out, in.hdr.nCmd, sizeof(Out));
		
		fSuccess = WriteFile( 
			hPipe,        // handle to pipe 
			&Out,         // buffer to write from 
			Out.nSize,    // number of bytes to write 
			&cbWritten,   // number of bytes written 
			NULL);        // not overlapped I/O 
		
	} else {
		MCHKHEAP;
		// Write the reply to the pipe. 
		fSuccess = WriteFile( 
			hPipe,        // handle to pipe 
			pOut,         // buffer to write from 
			pOut->hdr.nSize,  // number of bytes to write 
			&cbWritten,   // number of bytes written 
			NULL);        // not overlapped I/O 

		// ���������� ������
		if ((LPVOID)pOut != (LPVOID)gpStoredOutput) // ���� ��� �� ����������� �����
			ExecuteFreeResult(pOut);
	}

	if (pIn) { // �� �������������, ����, ����� ������� ������ �������� �� ����
		Free(pIn); pIn = NULL;
	}

	MCHKHEAP;
	//if (!fSuccess || pOut->hdr.nSize != cbWritten) break; 

// Flush the pipe to allow the client to read the pipe's contents 
// before disconnecting. Then disconnect the pipe, and close the 
// handle to this pipe instance. 

wrap: // Flush � Disconnect ������ ������
	FlushFileBuffers(hPipe); 
	DisconnectNamedPipe(hPipe);
	SafeCloseHandle(hPipe); 

	return 1;
}


BOOL GetAnswerToRequest(CESERVER_REQ& in, CESERVER_REQ** out)
{
	BOOL lbRc = FALSE;

	MCHKHEAP;

	switch (in.hdr.nCmd) {
		case CECMD_GETCONSOLEINFO:
		case CECMD_REQUESTCONSOLEINFO:
		{
			if (srv.szGuiPipeName[0] == 0) { // ��������� ���� � CVirtualConsole ��� ������ ���� �������
				wsprintf(srv.szGuiPipeName, CEGUIPIPENAME, L".", (DWORD)ghConWnd); // ��� gnSelfPID
			}

			_ASSERT(ghConOut && ghConOut!=INVALID_HANDLE_VALUE);
			if (ghConOut==NULL || ghConOut==INVALID_HANDLE_VALUE)
				return FALSE;

			ReloadFullConsoleInfo(TRUE);

			MCHKHEAP;

			// �� ������ �� GUI (GetAnswerToRequest)
			if (in.hdr.nCmd == CECMD_GETCONSOLEINFO) {
				//*out = CreateConsoleInfo(NULL, (in.hdr.nCmd == CECMD_GETFULLINFO));
				int nOutSize = sizeof(CESERVER_REQ_HDR) + sizeof(CESERVER_REQ_CONINFO_HDR);
				*out = ExecuteNewCmd(0,nOutSize);
				(*out)->ConInfo = *srv.pConsoleInfo;
			}

			MCHKHEAP;

			lbRc = TRUE;
		} break;
		//case CECMD_SETSIZE:
		case CECMD_SETSIZESYNC:
		case CECMD_SETSIZENOSYNC:
		case CECMD_CMDSTARTED:
		case CECMD_CMDFINISHED:
		{
			MCHKHEAP;
			int nOutSize = sizeof(CESERVER_REQ_HDR) + sizeof(CONSOLE_SCREEN_BUFFER_INFO) + sizeof(DWORD);
			*out = ExecuteNewCmd(0,nOutSize);
			if (*out == NULL) return FALSE;
			MCHKHEAP;
			if (in.hdr.nSize >= (sizeof(CESERVER_REQ_HDR) + sizeof(CESERVER_REQ_SETSIZE))) {
				USHORT nBufferHeight = 0;
				COORD  crNewSize = {0,0};
				SMALL_RECT rNewRect = {0};
				SHORT  nNewTopVisible = -1;
				//memmove(&nBufferHeight, in.Data, sizeof(USHORT));
				nBufferHeight = in.SetSize.nBufferHeight;
				//memmove(&crNewSize, in.Data+sizeof(USHORT), sizeof(COORD));
				crNewSize = in.SetSize.size;
				//memmove(&nNewTopVisible, in.Data+sizeof(USHORT)+sizeof(COORD), sizeof(SHORT));
				nNewTopVisible = in.SetSize.nSendTopLine;
				//memmove(&rNewRect, in.Data+sizeof(USHORT)+sizeof(COORD)+sizeof(SHORT), sizeof(SMALL_RECT));
				rNewRect = in.SetSize.rcWindow;

				MCHKHEAP;

				(*out)->hdr.nCmd = in.hdr.nCmd;
				// ��� ��������� ���� ��������� ��� ��������� � ExecuteNewCmd

				//#ifdef _DEBUG
				if (in.hdr.nCmd == CECMD_CMDFINISHED) {
					PRINT_COMSPEC(L"CECMD_CMDFINISHED, Set height to: %i\n", crNewSize.Y);
					DEBUGSTR(L"\n!!! CECMD_CMDFINISHED !!!\n\n");
					// ������� �����������
					TODO("����� ������ ������� �������")
					//if (srv.dwWinEventThread != 0)
					//	PostThreadMessage(srv.dwWinEventThread, srv.nMsgHookEnableDisable, TRUE, 0);
				} else if (in.hdr.nCmd == CECMD_CMDSTARTED) {
					PRINT_COMSPEC(L"CECMD_CMDSTARTED, Set height to: %i\n", nBufferHeight);
					DEBUGSTR(L"\n!!! CECMD_CMDSTARTED !!!\n\n");
					// ��������� �����������
					TODO("����� ������ ������� �������")
					//if (srv.dwWinEventThread != 0)
					//	PostThreadMessage(srv.dwWinEventThread, srv.nMsgHookEnableDisable, FALSE, 0);
				}
				//#endif

				if (in.hdr.nCmd == CECMD_CMDFINISHED) {
					// ��������� ������ ���� �������
					PRINT_COMSPEC(L"Storing long output\n", 0);
					CmdOutputStore();
					PRINT_COMSPEC(L"Storing long output (done)\n", 0);
				}

				MCHKHEAP;

				if (in.hdr.nCmd == CECMD_SETSIZESYNC) {
					ResetEvent(srv.hAllowInputEvent);
					srv.bInSyncResize = TRUE;
				}

				srv.nTopVisibleLine = nNewTopVisible;
				WARNING("���� ������ dwFarPID - ��� ���-�� ��� ���� ������ ����������?");
				SetConsoleSize(nBufferHeight, crNewSize, rNewRect, ":CECMD_SETSIZESYNC");

				WARNING("!! �� ����� �� ���������� �������� � ��������� ������ ��� �������� ����� ���������?");
				if (in.hdr.nCmd == CECMD_SETSIZESYNC) {
					CESERVER_REQ *pPlgIn = NULL, *pPlgOut = NULL;
					//TODO("���� �������������, ����� GUI ����������� ���������");
					if (in.SetSize.dwFarPID && !nBufferHeight) {
						// ������� ����� ��������� ����� ������ FAR�
						wchar_t szPipeName[128];
						wsprintf(szPipeName, CEPLUGINPIPENAME, L".", in.SetSize.dwFarPID);
						//DWORD nHILO = ((DWORD)crNewSize.X) | (((DWORD)(WORD)crNewSize.Y) << 16);
						//pPlgIn = ExecuteNewCmd(CMD_SETSIZE, sizeof(CESERVER_REQ_HDR)+sizeof(nHILO));
						pPlgIn = ExecuteNewCmd(CMD_REDRAWFAR, sizeof(CESERVER_REQ_HDR));
						//pPlgIn->dwData[0] = nHILO;
						pPlgOut = ExecuteCmd(szPipeName, pPlgIn, 500, ghConWnd);
						if (pPlgOut) ExecuteFreeResult(pPlgOut);
					}

					SetEvent(srv.hAllowInputEvent);
					srv.bInSyncResize = FALSE;
				}

				MCHKHEAP;

				if (in.hdr.nCmd == CECMD_CMDSTARTED) {
					// ������������ ����� ������� (������������ �����) ����� �������
					CmdOutputRestore();
				}
			}
			MCHKHEAP;
			
			PCONSOLE_SCREEN_BUFFER_INFO psc = &((*out)->SetSizeRet.SetSizeRet);
			MyGetConsoleScreenBufferInfo(ghConOut, psc);
			
			DWORD nPacketId = ++srv.nLastPacketID;
			(*out)->SetSizeRet.nNextPacketId = nPacketId;
			
			//srv.bForceFullSend = TRUE;
			SetEvent(srv.hRefreshEvent);

			MCHKHEAP;

			lbRc = TRUE;
		} break;

		case CECMD_GETOUTPUT:
		{
			if (gpStoredOutput) {
				DWORD nSize = sizeof(CESERVER_CONSAVE_HDR)
					+ min((int)gpStoredOutput->hdr.cbMaxOneBufferSize, 
					(gpStoredOutput->hdr.sbi.dwSize.X*gpStoredOutput->hdr.sbi.dwSize.Y*2));
				ExecutePrepareCmd(
					(CESERVER_REQ*)&(gpStoredOutput->hdr), CECMD_GETOUTPUT, nSize);

				*out = (CESERVER_REQ*)gpStoredOutput;

				lbRc = TRUE;
			}
		} break;
		
		case CECMD_ATTACH2GUI:
		{
			HWND hDc = Attach2Gui(1000);
			if (hDc != NULL) {
				int nOutSize = sizeof(CESERVER_REQ_HDR) + sizeof(DWORD);
				*out = ExecuteNewCmd(CECMD_ATTACH2GUI,nOutSize);
				if (*out != NULL) {
					// ����� �� ������������ "Press any key to close console"
					DisableAutoConfirmExit();
					//
					(*out)->dwData[0] = (DWORD)hDc;
					lbRc = TRUE;
				}
			}
		}  break;

		case CECMD_FARLOADED:
		{
			if (gbAutoDisableConfirmExit && srv.dwRootProcess == in.dwData[0]) {
				// FAR ��������� ����������, ������� ��� ��� �� � ������������� �������� ������� �� �����������
				DisableAutoConfirmExit();
			}
		} break;
		
		case CECMD_SHOWCONSOLE:
		{
			ShowWindow(ghConWnd, in.dwData[0]);
		} break;

		case CECMD_POSTCONMSG:
		{
			#ifdef _DEBUG
			WPARAM wParam = (WPARAM)in.Msg.wParam;
			LPARAM lParam = (LPARAM)in.Msg.lParam;
			
			if (in.Msg.nMsg == WM_INPUTLANGCHANGE || in.Msg.nMsg == WM_INPUTLANGCHANGEREQUEST) {
				unsigned __int64 l = lParam;
				wchar_t szDbg[255];
				wsprintf(szDbg, L"ConEmuC: %s(%s, CP:%i, HKL:0x%08I64X)\n",
					in.Msg.bPost ? L"PostMessage" : L"SendMessage",
					(in.Msg.nMsg == WM_INPUTLANGCHANGE) ? L"WM_INPUTLANGCHANGE" : L"WM_INPUTLANGCHANGEREQUEST",
					(DWORD)wParam, l);
				DEBUGLOGLANG(szDbg);
			}
			#endif

			if (in.Msg.bPost) {
				PostMessage(ghConWnd, in.Msg.nMsg, (WPARAM)in.Msg.wParam, (LPARAM)in.Msg.lParam);
			} else {
				LRESULT lRc = SendMessage(ghConWnd, in.Msg.nMsg, (WPARAM)in.Msg.wParam, (LPARAM)in.Msg.lParam);
				// ���������� ���������
				int nOutSize = sizeof(CESERVER_REQ_HDR) + sizeof(u64);
				*out = ExecuteNewCmd(CECMD_POSTCONMSG,nOutSize);
				if (*out != NULL) {
					(*out)->qwData[0] = lRc;
					lbRc = TRUE;
				}
			}
		} break;

		case CECMD_SETCONSOLECP:
		{
			// 1. ������������� ���������� ������ �����, ���������� � ��������
			_ASSERTE(!srv.hLockRefreshBegin && !srv.hLockRefreshReady);
			srv.hLockRefreshReady = CreateEvent(0,0,0,0);
			srv.hLockRefreshBegin = CreateEvent(0,0,0,0);
			while (WaitForSingleObject(srv.hLockRefreshReady, 10) == WAIT_TIMEOUT) {
				if (WaitForSingleObject(srv.hRefreshThread, 0) == WAIT_OBJECT_0)
					break; // ���� RefreshThread �����������!
			}

			// ����� � �� ����, �� �� ������ ������
			//MSectionLock CSCS; BOOL lbCSCS = FALSE;
			//lbCSCS = CSCS.Lock(&srv.cChangeSize, TRUE, 300);

			// 2. �������� ���� ������������ �������
			//ghConIn.Close();
			ghConOut.Close(); // ������� CECMD_SETCONSOLECP

			*out = ExecuteNewCmd(CECMD_SETCONSOLECP,sizeof(CESERVER_REQ_HDR)+sizeof(CESERVER_REQ_SETCONCP));

			// 3. ����������, ���������� SetConsoleCP, SetConsoleOutputCP
			BOOL bCpRc = FALSE;
			if (in.SetConCP.bSetOutputCP)
				bCpRc = SetConsoleOutputCP(in.SetConCP.nCP);
			else
				bCpRc = SetConsoleCP(in.SetConCP.nCP);

			if (*out != NULL) {
				(*out)->SetConCP.bSetOutputCP = bCpRc;
				(*out)->SetConCP.nCP = GetLastError();
			}

			// 4. ���������� ������ �����
			//if (lbCSCS) CSCS.Unlock();
			SetEvent(srv.hLockRefreshBegin);
			Sleep(10);
			SafeCloseHandle(srv.hLockRefreshReady);
			SafeCloseHandle(srv.hLockRefreshBegin);

			lbRc = TRUE;
		} break;

		case CECMD_SAVEALIASES:
		{
			//wchar_t* pszAliases; DWORD nAliasesSize;
			// ��������� ������
			DWORD nNewSize = in.hdr.nSize - sizeof(in.hdr);
			if (nNewSize > srv.nAliasesSize) {
				MCHKHEAP;
				wchar_t* pszNew = (wchar_t*)Alloc(nNewSize, 1);
				if (!pszNew) break; // �� ������� ������� ������
				MCHKHEAP;
				if (srv.pszAliases) Free(srv.pszAliases);
				MCHKHEAP;
				srv.pszAliases = pszNew;
				srv.nAliasesSize = nNewSize;
			}
			if (nNewSize > 0 && srv.pszAliases) {
				MCHKHEAP;
				memmove(srv.pszAliases, in.Data, nNewSize);
				MCHKHEAP;
			}
		} break;
		case CECMD_GETALIASES:
		{
			// ���������� ����������� ������
			int nOutSize = sizeof(CESERVER_REQ_HDR) + srv.nAliasesSize;
			*out = ExecuteNewCmd(CECMD_GETALIASES,nOutSize);
			if (*out != NULL) {
				if (srv.pszAliases && srv.nAliasesSize)
					memmove((*out)->Data, srv.pszAliases, srv.nAliasesSize);
				lbRc = TRUE;
			}
		} break;

		case CECMD_SETDONTCLOSE:
		{
			// ����� ������ � ���� ������� (�������� dir) ����������, �����
			// ������� ���������� �� ���������...
			gbAutoDisableConfirmExit = FALSE;
			gbAlwaysConfirmExit = TRUE;
		} break;

		case CECMD_ONACTIVATION:
		{
			if (srv.pConsoleInfo)
				srv.pConsoleInfo->bConsoleActive = in.dwData[0];
		} break;
	}
	
	if (gbInRecreateRoot) gbInRecreateRoot = FALSE;
	return lbRc;
}







// ��������� ���������� ������� WinApi (GetConsoleScreenBufferInfo), �� � ������ �������:
// 1. ��������� (�� ���� ��������) ������������ ����������� ����
// 2. ������������ srWindow: ���������� �������������� ���������,
//    � ���� �� ��������� "�������� �����" - �� � ������������.
BOOL MyGetConsoleScreenBufferInfo(HANDLE ahConOut, PCONSOLE_SCREEN_BUFFER_INFO apsc)
{
	BOOL lbRc = FALSE;

	//CSection cs(NULL,NULL);
	//MSectionLock CSCS;
	//if (gnRunMode == RM_SERVER)
		//CSCS.Lock(&srv.cChangeSize);
		//cs.Enter(&srv.csChangeSize, &srv.ncsTChangeSize);

	if (gnRunMode == RM_SERVER && ghConEmuWnd && IsWindow(ghConEmuWnd)) // ComSpec ���� ������ �� ������!
	{
		// ���� ���� �������� ����� ������������, ����� ���������� ���� ������ - ������ �������� �� �����
		if (IsZoomed(ghConWnd)) {
			SendMessage(ghConWnd, WM_SYSCOMMAND, SC_RESTORE, 0);

			DWORD dwStartTick = GetTickCount();
			do {
				Sleep(20); // �������� ����, �� �� ������ �������
			} while (IsZoomed(ghConWnd) && (GetTickCount()-dwStartTick)<=1000);
			Sleep(20); // � ��� ����-����, ����� ������� �����������

			// ������ ����� ������� (������ �� ���������) ������ ������ �������

			// ���� ����� �� ������� - ������ ������� ������ ���������
			RECT rcConPos;
			GetWindowRect(ghConWnd, &rcConPos);
			MoveWindow(ghConWnd, rcConPos.left, rcConPos.top, 1, 1, 1);
			if (gnBufferHeight == 0)
			{
				//specified width and height cannot be less than the width and height of the console screen buffer's window
				lbRc = SetConsoleScreenBufferSize(ghConOut, gcrBufferSize);
			} else {
				// ������� ������ ��� BufferHeight
				COORD crHeight = {gcrBufferSize.X, gnBufferHeight};
				MoveWindow(ghConWnd, rcConPos.left, rcConPos.top, 1, 1, 1);
				lbRc = SetConsoleScreenBufferSize(ghConOut, crHeight); // � �� crNewSize - ��� "�������" �������
			}

			// � ������� ��� ������� �������������, ������� ��� ������� � ��������� ��� (�������� ���)
			SetConsoleWindowInfo(ghConOut, TRUE, &srv.sbi.srWindow);
		}
	}

	CONSOLE_SCREEN_BUFFER_INFO csbi = {sizeof(CONSOLE_SCREEN_BUFFER_INFO)};

	lbRc = GetConsoleScreenBufferInfo(ahConOut, &csbi);
	// 
	_ASSERTE((csbi.srWindow.Bottom-csbi.srWindow.Top)<200);
	if (lbRc && gnRunMode == RM_SERVER) // ComSpec ���� ������ �� ������!
	{
				// ���������� � SetConsoleSize
				//     if (gnBufferHeight) {
				//// ���� �� ����� � ������ BufferHeight - ����� ����������������� ������ (����� ��� ���� �������?)
				//         if (gnBufferHeight <= (csbi.dwMaximumWindowSize.Y * 12 / 10))
				//             gnBufferHeight = max(300, (SHORT)(csbi.dwMaximumWindowSize.Y * 12 / 10));
				//     }

		// ���� ��������� ���� �� ������ - �� ����������� ������ ��, ����� ��� ������� FAR
		// ���������� ������ � ������� �������

		// ����� � ������ �������
		BOOL lbNeedCorrect = FALSE;
		if (csbi.srWindow.Left > 0) {
			lbNeedCorrect = TRUE; csbi.srWindow.Left = 0;
		}
		if ((csbi.srWindow.Right+1) < csbi.dwSize.X) {
			lbNeedCorrect = TRUE; csbi.srWindow.Right = (csbi.dwSize.X - 1);
		}

		BOOL lbBufferHeight = FALSE;
		if (csbi.dwSize.Y >= (csbi.dwMaximumWindowSize.Y * 12 / 10))
			lbBufferHeight = TRUE;

		if (!lbBufferHeight) {
			_ASSERTE((csbi.srWindow.Bottom-csbi.srWindow.Top)<200);

			if (csbi.srWindow.Top > 0) {
				lbNeedCorrect = TRUE; csbi.srWindow.Top = 0;
			}
			if ((csbi.srWindow.Bottom+1) < csbi.dwSize.Y) {
				lbNeedCorrect = TRUE; csbi.srWindow.Bottom = (csbi.dwSize.Y - 1);
			}
		}
		if (CorrectVisibleRect(&csbi))
			lbNeedCorrect = TRUE;
		if (lbNeedCorrect) {
			lbRc = SetConsoleWindowInfo(ghConOut, TRUE, &csbi.srWindow);
			lbRc = GetConsoleScreenBufferInfo(ahConOut, &csbi);
		}
	}

	// ���������� (��������) ����������������� ������
	*apsc = csbi;

	//cs.Leave();
	//CSCS.Unlock();

	#ifdef _DEBUG
	if ((csbi.srWindow.Bottom - csbi.srWindow.Top)>csbi.dwMaximumWindowSize.Y) {
		// ������������, ����� ���� �� ����� - ��� ������ ���� �������?
		_ASSERTE((csbi.srWindow.Bottom - csbi.srWindow.Top)<csbi.dwMaximumWindowSize.Y);
	}
	#endif

	return lbRc;
}



// BufferHeight  - ������ ������ (0 - ��� ���������)
// crNewSize     - ������ ���� (������ ���� == ������ ������)
// rNewRect      - ��� (BufferHeight!=0) ���������� new upper-left and lower-right corners of the window
BOOL SetConsoleSize(USHORT BufferHeight, COORD crNewSize, SMALL_RECT rNewRect, LPCSTR asLabel)
{
	_ASSERTE(ghConWnd);
	if (!ghConWnd) return FALSE;

//#ifdef _DEBUG
//	if (gnRunMode != RM_SERVER || !srv.bDebuggerActive)
//	{
//		BOOL bFarInExecute = WaitForSingleObject(ghFarInExecuteEvent, 0) == WAIT_OBJECT_0;
//		if (BufferHeight) {
//			if (!bFarInExecute) {
//				_ASSERTE(BufferHeight && bFarInExecute);
//			}
//		}
//	}
//#endif

	DWORD dwCurThId = GetCurrentThreadId();
	DWORD dwWait = 0;

	if (gnRunMode == RM_SERVER) {
		// �������� ��, ��� ��������� ��� ��������� ������. ����������
		srv.nReqSizeBufferHeight = BufferHeight;
		srv.crReqSizeNewSize = crNewSize;
		srv.rReqSizeNewRect = rNewRect;
		srv.sReqSizeLabel = asLabel;

		if (srv.dwRefreshThread && dwCurThId != srv.dwRefreshThread) {
			ResetEvent(srv.hReqSizeChanged);
			srv.nRequestChangeSize++;
			
			//dwWait = WaitForSingleObject(srv.hReqSizeChanged, REQSIZE_TIMEOUT);
			// ��������, ���� ��������� RefreshThread
			HANDLE hEvents[2] = {ghQuitEvent, srv.hReqSizeChanged};
			DWORD nSizeTimeout = REQSIZE_TIMEOUT;
			#ifdef _DEBUG
			if (IsDebuggerPresent())
				nSizeTimeout = INFINITE;
			#endif
			dwWait = WaitForMultipleObjects ( 2, hEvents, FALSE, nSizeTimeout );
			if (srv.nRequestChangeSize > 0) {
				srv.nRequestChangeSize --;
			} else {
				_ASSERTE(srv.nRequestChangeSize>0);
			}
			if (dwWait == WAIT_OBJECT_0) {
				// ghQuitEvent !!
				return FALSE;
			}
			if (dwWait == (WAIT_OBJECT_0+1)) {
				return TRUE;
			}
			// ?? ����� ���� ����� ����� �����������?
			return FALSE;
		}
	}

	//CSection cs(NULL,NULL);
	//MSectionLock CSCS;
	//if (gnRunMode == RM_SERVER)
	//	CSCS.Lock(&srv.cChangeSize, TRUE, 10000);
	//    //cs.Enter(&srv.csChangeSize, &srv.ncsTChangeSize);
	
	if (ghLogSize) LogSize(&crNewSize, asLabel);

	_ASSERTE(crNewSize.X>=MIN_CON_WIDTH && crNewSize.Y>=MIN_CON_HEIGHT);

	// �������� ������������ �������
	if (crNewSize.X</*4*/MIN_CON_WIDTH)
		crNewSize.X = /*4*/MIN_CON_WIDTH;
	if (crNewSize.Y</*3*/MIN_CON_HEIGHT)
		crNewSize.Y = /*3*/MIN_CON_HEIGHT;

	BOOL lbNeedChange = TRUE;
	CONSOLE_SCREEN_BUFFER_INFO csbi = {{0,0}};
	if (MyGetConsoleScreenBufferInfo(ghConOut, &csbi)) {
		lbNeedChange = (csbi.dwSize.X != crNewSize.X) || (csbi.dwSize.Y != crNewSize.Y);
#ifdef _DEBUG
		if (!lbNeedChange) {
			BufferHeight = BufferHeight;
		}
#endif
	}

	COORD crMax = GetLargestConsoleWindowSize(ghConOut);
	if (crMax.X && crNewSize.X > crMax.X)
		crNewSize.X = crMax.X;
	if (crMax.Y && crNewSize.Y > crMax.Y)
		crNewSize.Y = crMax.Y;

	// ������ ��� ����� MyGetConsoleScreenBufferInfo, �.�. ��������� ��������� ������� ���� 
	// ��� ������ ������������ �� gnBufferHeight
	gnBufferHeight = BufferHeight;
	gcrBufferSize = crNewSize;
	_ASSERTE(gcrBufferSize.Y<200);

	if (gnBufferHeight) {
		// � ������ BufferHeight - ������ ������ ���� ������ ����������� ������� ���� �������
		// ����� �� ���������� ��� ��������� "�������� �� ��� �����"...
		if (gnBufferHeight <= (csbi.dwMaximumWindowSize.Y * 12 / 10))
			gnBufferHeight = max(300, (SHORT)(csbi.dwMaximumWindowSize.Y * 12 / 10));

		// � ������ cmd ����� �������� ������������ FPS
		srv.dwLastUserTick = GetTickCount() - USER_IDLE_TIMEOUT - 1;
	}

	RECT rcConPos = {0};
	GetWindowRect(ghConWnd, &rcConPos);

	BOOL lbRc = TRUE;
	//DWORD nWait = 0;

	//if (srv.hChangingSize) {
	//    nWait = WaitForSingleObject(srv.hChangingSize, 300);
	//    _ASSERTE(nWait == WAIT_OBJECT_0);
	//    ResetEvent(srv.hChangingSize);
	//}

	// case: simple mode
	if (BufferHeight == 0)
	{
		if (lbNeedChange) {
			DWORD dwErr = 0;
			// ���� ����� �� ������� - ������ ������� ������ ���������
			//if (crNewSize.X < csbi.dwSize.X || crNewSize.Y < csbi.dwSize.Y)
			if (crNewSize.X <= (csbi.srWindow.Right-csbi.srWindow.Left) || crNewSize.Y <= (csbi.srWindow.Bottom-csbi.srWindow.Top))
			{
				//MoveWindow(ghConWnd, rcConPos.left, rcConPos.top, 1, 1, 1);
				rNewRect.Left = 0; rNewRect.Top = 0;
				rNewRect.Right = min((crNewSize.X - 1),(csbi.srWindow.Right-csbi.srWindow.Left));
				rNewRect.Bottom = min((crNewSize.Y - 1),(csbi.srWindow.Bottom-csbi.srWindow.Top));
				if (!SetConsoleWindowInfo(ghConOut, TRUE, &rNewRect))
					MoveWindow(ghConWnd, rcConPos.left, rcConPos.top, 1, 1, 1);
			}
			//specified width and height cannot be less than the width and height of the console screen buffer's window
			lbRc = SetConsoleScreenBufferSize(ghConOut, crNewSize);
			if (!lbRc) dwErr = GetLastError();
			//TODO: � ���� ������ ������ ���� ������� �� ������� ������?
			//WARNING("�������� ��� �����");
			//MoveWindow(ghConWnd, rcConPos.left, rcConPos.top, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), 1);
			rNewRect.Left = 0; rNewRect.Top = 0;
			rNewRect.Right = crNewSize.X - 1;
			rNewRect.Bottom = crNewSize.Y - 1;
			SetConsoleWindowInfo(ghConOut, TRUE, &rNewRect);
		}

	} else {
		// ������� ������ ��� BufferHeight
		COORD crHeight = {crNewSize.X, BufferHeight};

		/*if (gOSVer.dwMajorVersion == 6 && gOSVer.dwMinorVersion == 1) {
			SetConsoleBufferSize(ghConWnd, crNewSize.X, crNewSize.Y, BufferHeight);
			Sleep(10);
			CONSOLE_SCREEN_BUFFER_INFO csbi2 = {{0,0}};
			DWORD dwStart = GetTickCount(), dwWait = 200;
			do {
				if (GetConsoleScreenBufferInfo(ghConOut, &csbi2)) {
					if (csbi2.dwSize.X == crNewSize.X && csbi2.dwSize.Y == BufferHeight) {
						break;
					}
				}
				Sleep(10);
			} while ((GetTickCount() - dwStart) < dwWait);

		} else {*/
		// ���� ����� �� ������� - ������ ������� ������ ���������
		if (crNewSize.X <= (csbi.srWindow.Right-csbi.srWindow.Left) || crNewSize.Y <= (csbi.srWindow.Bottom-csbi.srWindow.Top))
		{
			//MoveWindow(ghConWnd, rcConPos.left, rcConPos.top, 1, 1, 1);
			rNewRect.Left = 0; rNewRect.Top = 0;
			rNewRect.Right = min((crNewSize.X - 1),(csbi.srWindow.Right-csbi.srWindow.Left));
			rNewRect.Bottom = min((crNewSize.Y - 1),(csbi.srWindow.Bottom-csbi.srWindow.Top));
			if (!SetConsoleWindowInfo(ghConOut, TRUE, &rNewRect))
				MoveWindow(ghConWnd, rcConPos.left, rcConPos.top, 1, 1, 1);
		}

		//MoveWindow(ghConWnd, rcConPos.left, rcConPos.top, 1, 1, 1);

		//WaitForSingleObject(ghConOut, 200);
		//Sleep(100);
		/*if (gOSVer.dwMajorVersion == 6 && gOSVer.dwMinorVersion == 1) {
			const SHORT nMaxBuf = 600;
			if (crHeight.Y >= nMaxBuf)
				crHeight.Y = nMaxBuf;
		}*/

		lbRc = SetConsoleScreenBufferSize(ghConOut, crHeight); // � �� crNewSize - ��� "�������" �������

		//}
		//������ ���������� ������ �� ������!
		//RECT rcCurConPos = {0};
		//GetWindowRect(ghConWnd, &rcCurConPos); //X-Y �����, �� ������ - ������
		//MoveWindow(ghConWnd, rcCurConPos.left, rcCurConPos.top, GetSystemMetrics(SM_CXSCREEN), rcConPos.bottom-rcConPos.top, 1);

		rNewRect.Left = 0;
		rNewRect.Right = crHeight.X-1;
		rNewRect.Bottom = min( (crHeight.Y-1), (rNewRect.Top+gcrBufferSize.Y-1) );
		_ASSERTE((rNewRect.Bottom-rNewRect.Top)<200);
		SetConsoleWindowInfo(ghConOut, TRUE, &rNewRect);
	}

	//if (srv.hChangingSize) { // �� ����� ������� ConEmuC
	//    SetEvent(srv.hChangingSize);
	//}
	
	if (gnRunMode == RM_SERVER) {
		//srv.bForceFullSend = TRUE;
		SetEvent(srv.hRefreshEvent);
	}

	//cs.Leave();
	//CSCS.Unlock();

	return lbRc;
}

BOOL WINAPI HandlerRoutine(DWORD dwCtrlType)
{
	//PRINT_COMSPEC(L"HandlerRoutine triggered. Event type=%i\n", dwCtrlType);
	if (dwCtrlType >= CTRL_CLOSE_EVENT && dwCtrlType <= CTRL_SHUTDOWN_EVENT) {
		PRINT_COMSPEC(L"Console about to be closed\n", 0);
		gbInShutdown = TRUE;
		
		// ���������� ��������, ����� ������������ ������� ���� ����������	
		if (srv.bDebuggerActive) {
			if (pfnDebugActiveProcessStop) pfnDebugActiveProcessStop(srv.dwRootProcess);
			srv.bDebuggerActive = FALSE;
		}
	}
	/*SafeCloseHandle(ghLogSize);
	if (wpszLogSizeFile) {
		DeleteFile(wpszLogSizeFile);
		Free(wpszLogSizeFile); wpszLogSizeFile = NULL;
	}*/

	return TRUE;
}

int GetProcessCount(DWORD *rpdwPID, UINT nMaxCount)
{
	if (!rpdwPID || !nMaxCount) {
		_ASSERTE(rpdwPID && nMaxCount);
		return srv.nProcessCount;
	}

	MSectionLock CS;
	if (!CS.Lock(srv.csProc, 200)) {
		// ���� �� ������� ������������� ���������� - ������ ������ ����
		*rpdwPID = gnSelfPID;
		return 1;
	}

	UINT nSize = srv.nProcessCount;
	if (nSize > nMaxCount) {
		memset(rpdwPID, 0, sizeof(DWORD)*nMaxCount);
		rpdwPID[0] = gnSelfPID;

		for (int i1=0, i2=(nMaxCount-1); i1<(int)nSize && i2>0; i1++, i2--)
			rpdwPID[i2] = srv.pnProcesses[i1];

		nSize = nMaxCount;

	} else {
		memmove(rpdwPID, srv.pnProcesses, sizeof(DWORD)*nSize);

		for (UINT i=nSize; i<nMaxCount; i++)
			rpdwPID[i] = 0;
	}

	_ASSERTE(rpdwPID[0]);

	return nSize;

	/*
	//DWORD dwErr = 0; BOOL lbRc = FALSE;
	DWORD *pdwPID = NULL; int nCount = 0, i;
	EnterCriticalSection(&srv.csProc);
	nCount = srv.nProcesses.size();
	if (nCount > 0 && rpdwPID) {
		pdwPID = (DWORD*)Alloc(nCount, sizeof(DWORD));
		_ASSERTE(pdwPID!=NULL);
		if (pdwPID) {
			std::vector<DWORD>::iterator iter = srv.nProcesses.begin();
			i = 0;
			while (iter != srv.nProcesses.end()) {
				pdwPID[i++] = *iter;
				iter ++;
			}
		}
	}
	LeaveCriticalSection(&srv.csProc);
	if (rpdwPID)
		*rpdwPID = pdwPID;
	return nCount;
	*/
}

const wchar_t* PointToName(const wchar_t* asFullPath)
{
	const wchar_t* pszFile = wcsrchr(asFullPath, L'\\');
	if (!pszFile) pszFile = asFullPath; else pszFile++;
	return pszFile;
}

#ifdef CRTPRINTF
WARNING("����� ���������... �������� �� wvsprintf");
//void _printf(LPCSTR asFormat, ...)
//{
//    va_list argList; va_start(argList, an_StrResId);
//    char szError[2000]; -- ������ ����� ������ ����� %s
//    wvsprintf(szError, asFormat, argList);
//}

void _printf(LPCSTR asFormat, DWORD dw1, DWORD dw2, LPCWSTR asAddLine)
{
	char szError[MAX_PATH];
	wsprintfA(szError, asFormat, dw1, dw2);
	_printf(szError);
	if (asAddLine) {
		_wprintf(asAddLine);
		_printf("\n");
	}
}

void _printf(LPCSTR asFormat, DWORD dwErr, LPCWSTR asAddLine)
{
	char szError[MAX_PATH];
	wsprintfA(szError, asFormat, dwErr);
	_printf(szError);
	_wprintf(asAddLine);
	_printf("\n");
}

void _printf(LPCSTR asFormat, DWORD dwErr)
{
	char szError[MAX_PATH];
	wsprintfA(szError, asFormat, dwErr);
	_printf(szError);
}

void _printf(LPCSTR asBuffer)
{
	if (!asBuffer) return;
	int nAllLen = lstrlenA(asBuffer);
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwWritten = 0;
	WriteFile(hOut, asBuffer, nAllLen, &dwWritten, 0);
}

void _wprintf(LPCWSTR asBuffer)
{
	if (!asBuffer) return;
	int nAllLen = lstrlenW(asBuffer);
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwWritten = 0;
	UINT nOldCP = GetConsoleOutputCP();
	char* pszOEM = (char*)malloc(nAllLen+1);
	if (pszOEM) {
		WideCharToMultiByte(nOldCP,0, asBuffer, nAllLen, pszOEM, nAllLen, 0,0);
		pszOEM[nAllLen] = 0;
		WriteFile(hOut, pszOEM, nAllLen, &dwWritten, 0);
		free(pszOEM);
	} else {
		WriteFile(hOut, asBuffer, nAllLen*2, &dwWritten, 0);
	}
}
#endif

void DisableAutoConfirmExit()
{
	gbAutoDisableConfirmExit = FALSE; gbAlwaysConfirmExit = FALSE;
	// ������ nProcessStartTick �� �����. �������� ������ �� �������
	//srv.nProcessStartTick = GetTickCount() - 2*CHECK_ROOTSTART_TIMEOUT;
}

//BOOL IsUserAdmin()
//{
//	OSVERSIONINFO osv = {sizeof(OSVERSIONINFO)};
//	GetVersionEx(&osv);
//	// ��������� ����� ������ ��� �����, ����� �� XP ������ "���" �� �����������
//	if (osv.dwMajorVersion < 6)
//		return FALSE;
//
//	BOOL b;
//	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
//	PSID AdministratorsGroup;
//
//	b = AllocateAndInitializeSid(
//		&NtAuthority,
//		2,
//		SECURITY_BUILTIN_DOMAIN_RID,
//		DOMAIN_ALIAS_RID_ADMINS,
//		0, 0, 0, 0, 0, 0,
//		&AdministratorsGroup); 
//	if(b) 
//	{
//		if (!CheckTokenMembership(NULL, AdministratorsGroup, &b)) 
//		{
//			b = FALSE;
//		}
//		FreeSid(AdministratorsGroup);
//	}
//	return(b);
//}

WARNING("BUGBUG: x64 US-Dvorak");
void CheckKeyboardLayout()
{
	if (pfnGetConsoleKeyboardLayoutName) {
		wchar_t szCurKeybLayout[32];
		// ���������� ������ � ���� "00000419" -- ����� ��� 16 ����?
		if (pfnGetConsoleKeyboardLayoutName(szCurKeybLayout)) {
			if (lstrcmpW(szCurKeybLayout, srv.szKeybLayout)) {
				#ifdef _DEBUG
				wchar_t szDbg[128];
				wsprintfW(szDbg, 
					L"ConEmuC: InputLayoutChanged (GetConsoleKeyboardLayoutName returns) '%s'\n", 
					szCurKeybLayout);
				OutputDebugString(szDbg);
				#endif
				// ��������
				lstrcpyW(srv.szKeybLayout, szCurKeybLayout);
				// ������� � GUI
				wchar_t *pszEnd = szCurKeybLayout+8;
				WARNING("BUGBUG: 16 ���� �� ������");
				// LayoutName: "00000409", "00010409", ...
				// � HKL �� ���� ����������, ��� ��� �������� DWORD
				// HKL � x64 �������� ���: "0x0000000000020409", "0xFFFFFFFFF0010409"
				DWORD dwLayout = wcstol(szCurKeybLayout, &pszEnd, 16);
				CESERVER_REQ* pIn = ExecuteNewCmd(CECMD_LANGCHANGE,sizeof(CESERVER_REQ_HDR)+sizeof(DWORD));
				if (pIn) {
					//memmove(pIn->Data, &dwLayout, 4);
					pIn->dwData[0] = dwLayout;

					CESERVER_REQ* pOut = NULL;
					pOut = ExecuteGuiCmd(ghConWnd, pIn, ghConWnd);
					if (pOut) ExecuteFreeResult(pOut);
					ExecuteFreeResult(pIn);
				}
			}
		}
	}
}


LPVOID Alloc(size_t nCount, size_t nSize)
{
	#ifdef _DEBUG
	//HeapValidate(ghHeap, 0, NULL);
	#endif

	size_t nWhole = nCount * nSize;
	_ASSERTE(nWhole>0);
	LPVOID ptr = HeapAlloc ( ghHeap, HEAP_GENERATE_EXCEPTIONS|HEAP_ZERO_MEMORY, nWhole );

	#ifdef HEAP_LOGGING
	wchar_t szDbg[64];
	wsprintfW(szDbg, L"%i: ALLOCATED   0x%08X..0x%08X   (%i bytes)\n", GetCurrentThreadId(), (DWORD)ptr, ((DWORD)ptr)+nWhole, nWhole);
	DEBUGSTR(szDbg);
	#endif

	#ifdef _DEBUG
	HeapValidate(ghHeap, 0, NULL);
	if (ptr) {
		gnHeapUsed += nWhole;
		if (gnHeapMax < gnHeapUsed)
			gnHeapMax = gnHeapUsed;
	}
	#endif

	return ptr;
}

void Free(LPVOID ptr)
{
	if (ptr && ghHeap) {
		#ifdef _DEBUG
		//HeapValidate(ghHeap, 0, NULL);
		size_t nMemSize = HeapSize(ghHeap, 0, ptr);
		#endif
		#ifdef HEAP_LOGGING
		wchar_t szDbg[64];
		wsprintfW(szDbg, L"%i: FREE BLOCK  0x%08X..0x%08X   (%i bytes)\n", GetCurrentThreadId(), (DWORD)ptr, ((DWORD)ptr)+nMemSize, nMemSize);
		DEBUGSTR(szDbg);
		#endif

		HeapFree ( ghHeap, 0, ptr );

		#ifdef _DEBUG
		HeapValidate(ghHeap, 0, NULL);
		if (gnHeapUsed > nMemSize)
			gnHeapUsed -= nMemSize;
		#endif
	}
}

/* ������������ ��� extern � ConEmuCheck.cpp */
LPVOID _calloc(size_t nCount,size_t nSize) {
	return Alloc(nCount,nSize);
}
LPVOID _malloc(size_t nCount) {
	return Alloc(nCount,1);
}
void   _free(LPVOID ptr) {
	Free(ptr);
}