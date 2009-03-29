/*   PictureView wrapper  for ConEmu   */
/*  Copyright (c) Maxim Moysyuk, 2009  */

#include <windows.h>
#include <TCHAR.H>
#include "PicViewWrapper.h"

extern "C" {
#include "../../common/ConEmuCheck.h"
}

/* FAR */
PluginStartupInfo psi;
FarStandardFunctions fsf;
FWrapperAdvControl fWrapperAdvControl=NULL;

/* PictureView */
HMODULE hPicViewDll=NULL;
HMODULE hEmuPicDll=NULL;
FClosePlugin fClosePlugin=NULL;
FConfigure fConfigure=NULL;
FExitFAR fExitFAR=NULL;
FGetMinFarVersion fGetMinFarVersion=NULL;
FGetPluginInfo fGetPluginInfo=NULL;
FOpenFilePlugin fOpenFilePlugin=NULL;
FOpenPlugin fOpenPlugin=NULL;
FProcessEditorEvent fProcessEditorEvent=NULL;
FProcessViewerEvent fProcessViewerEvent=NULL;
FSetStartupInfo fSetStartupInfo=NULL;

/* Local */
HINSTANCE ghInstance = NULL;
TCHAR gsTermMsg[128];
TCHAR gsLoadMsg[320];

/* ConEmu */
HWND ghConEmu = NULL;
//HMODULE ghConEmuDll=NULL;
//typedef HWND (WINAPI* FGetFarHWND2)(BOOL abConEmuOnly);
//FGetFarHWND2 GetFarHWND2=NULL;
BOOL gbOldConEmu=FALSE;
BOOL gbConEmuChecked=FALSE, gbOldConEmuWarned=FALSE;
BOOL IsOldConEmu();
//typedef HWND (WINAPI* FIsTerminalMode)();
//FIsTerminalMode IsTerminalMode=NULL;


BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        {
            // Init variables
            ghInstance = (HINSTANCE)hModule;
            gsTermMsg[0] = 0; gsLoadMsg[0] = 0;
            memset(&psi, 0, sizeof(psi));
            memset(&fsf, 0, sizeof(fsf));

            // ���������������� ����� ������ � ��������� "��������"
            IsOldConEmu();
            
            // Check Terminal mode
            TCHAR szVarValue[MAX_PATH];
            szVarValue[0] = 0; gsTermMsg[0] = 0;
            if (GetEnvironmentVariable(_T("TERM"), szVarValue, 63)) {
                lstrcpy(gsTermMsg, _T("PictureView wrapper\nPicture viewing is not available\nin terminal mode ("));
                lstrcat(gsTermMsg, szVarValue);
                lstrcat(gsTermMsg, _T(")"));
            }

            
            // Init PictureView plugin
            if (!hPicViewDll) {
                TCHAR szModulePath[MAX_PATH+1], szEmuPic[MAX_PATH+1], *pszSlash;
                int nLen = 0;
                if ((nLen=GetModuleFileName(ghInstance, szModulePath, MAX_PATH))>0)
                {
	                // ��� UnderScrore 0PictureView.dl_ �� ����� �������������� � EmuPic.dll
	                _tcscpy(szEmuPic, szModulePath);
	                pszSlash = szEmuPic+nLen-1;
	                while (pszSlash>szEmuPic && *(pszSlash-1)!=_T('\\')) pszSlash--;
	                _tcscpy(pszSlash, _T("EmuPic.dll"));
	                hEmuPicDll = LoadLibrary ( szEmuPic );
	                if (!hEmuPicDll) {
	                    if (!gsTermMsg[0]) {
		                    DWORD dwErr;
		                    dwErr = GetLastError();
		                    wsprintf(gsLoadMsg, "PictureView wrapper\nCan't load library!\n%s\nLastError=0x%08x", szEmuPic, dwErr);
	                        //MessageBox(ghConEmu, szError, _T("PictureView wrapper"), MB_ICONSTOP);
	                    }
                        return TRUE; // ����� FAR � Windows7 ������ �� ���������� ������
	                }
	                
                    if (szModulePath[nLen-1]!=_T('L') && szModulePath[nLen-1]!=_T('l')) {
	                    if (!gsTermMsg[0]) {
		                    DWORD dwErr;
		                    dwErr = GetLastError();
		                    wsprintf(gsLoadMsg, "PictureView wrapper\nInvalid file name!\n%s\n*.dll expected", szModulePath);
	                        //MessageBox(ghConEmu, szError, _T("PictureView wrapper"), MB_ICONSTOP);
	                    }
                        return TRUE; // ����� FAR � Windows7 ������ �� ���������� ������
                    }
                    szModulePath[nLen-1] = '_';
                    hPicViewDll = LoadLibrary(szModulePath);
                    if (!hPicViewDll) {
	                    if (!gsTermMsg[0]) {
		                    DWORD dwErr;
		                    dwErr = GetLastError();
		                    wsprintf(gsLoadMsg, "PictureView wrapper\nCan't load library!\n%s\nLastError=0x%08X", szModulePath, dwErr);
	                        //MessageBox(ghConEmu, szError, _T("PictureView wrapper"), MB_ICONSTOP);
	                    }
                        return TRUE; // ����� FAR � Windows7 ������ �� ���������� ������
                    }
                }
            }
            if (hPicViewDll) {
	            fClosePlugin=(FClosePlugin)GetProcAddress(hPicViewDll,"ClosePlugin");
	            fConfigure=(FConfigure)GetProcAddress(hPicViewDll,"Configure");
	            fExitFAR=(FExitFAR)GetProcAddress(hPicViewDll,"ExitFAR");
	            fGetMinFarVersion=(FGetMinFarVersion)GetProcAddress(hPicViewDll,"GetMinFarVersion");
	            fGetPluginInfo=(FGetPluginInfo)GetProcAddress(hPicViewDll,"GetPluginInfo");
	            fOpenFilePlugin=(FOpenFilePlugin)GetProcAddress(hPicViewDll,"OpenFilePlugin");
	            fOpenPlugin=(FOpenPlugin)GetProcAddress(hPicViewDll,"OpenPlugin");
	            fProcessEditorEvent=(FProcessEditorEvent)GetProcAddress(hPicViewDll,"ProcessEditorEvent");
	            fProcessViewerEvent=(FProcessViewerEvent)GetProcAddress(hPicViewDll,"ProcessViewerEvent");
	            fSetStartupInfo=(FSetStartupInfo)GetProcAddress(hPicViewDll,"SetStartupInfo");
	        }
        } break;
    case DLL_PROCESS_DETACH:
        {
            if (hPicViewDll) {
                FreeLibrary(hPicViewDll);
                hPicViewDll = NULL;
                HMODULE hEmuPic = GetModuleHandle(_T("EmuPic.dll"));
                //if (!hEmuPic) FreeLibrary(hEmuPic);
            }
        } break;
    }
    return TRUE;
}


void WINAPI ClosePlugin(
  HANDLE hPlugin
)
{
    if (hPicViewDll && fClosePlugin)
        fClosePlugin(hPlugin);
}

int WINAPI Configure(
  int ItemNumber
)
{
    if (/*!gbOldConEmu &&*/ hPicViewDll && fConfigure)
        return fConfigure(ItemNumber);
    return FALSE;
}

void WINAPI ExitFAR( void )
{
    if (hPicViewDll && fExitFAR)
        fExitFAR();
}

int WINAPI GetMinFarVersion(void)
{
    if (hPicViewDll && fGetMinFarVersion)
        return fGetMinFarVersion();
    return MAKEFARVERSION(1,71,2470);
}

void WINAPI GetPluginInfo(
  struct PluginInfo *Info
)
{
    if (hPicViewDll && fGetPluginInfo)
        fGetPluginInfo(Info);
}

HANDLE WINAPI OpenFilePlugin(
  char *Name,
  const unsigned char *Data,
  int DataSize
)
{
    HANDLE hRc = INVALID_HANDLE_VALUE;
    if (/*!gbOldConEmu &&*/ hPicViewDll && fOpenFilePlugin) {
        if (gsTermMsg[0]) {
            TCHAR *pszExt = NULL;
            // PicView ����� �������������� � �� Enter/CtrlPgDn, ����� �������� ������ �� "���������" �����, ��������� ������ ������������
            if (Name && *Name) {
                for (pszExt=Name+lstrlen(Name)-1; pszExt>=Name && *pszExt!=_T('\\'); pszExt--) {
                    if (*pszExt==_T('.')) {
                        pszExt++;
                        if (lstrcmpi(pszExt, _T("jpg")) && lstrcmpi(pszExt, _T("jpeg")) && lstrcmpi(pszExt, _T("jfif")) &&
                            lstrcmpi(pszExt, _T("tif")) && lstrcmpi(pszExt, _T("tiff")) &&
                            lstrcmpi(pszExt, _T("bmp")) && lstrcmpi(pszExt, _T("dib")) &&
                            lstrcmpi(pszExt, _T("wmf")) && lstrcmpi(pszExt, _T("emf")) &&
                            lstrcmpi(pszExt, _T("pcx")) &&
                            lstrcmpi(pszExt, _T("gif")) &&
                            1)
                            pszExt=NULL;
                        break;
                    }
                }

            }
            if (pszExt && psi.Message) {
                psi.Message(psi.ModuleNumber, FMSG_MB_OK|FMSG_ALLINONE, NULL, 
                    (const TCHAR *const *)gsTermMsg, 0, 0);
            }
        } else {
            hRc = fOpenFilePlugin(Name, Data, DataSize);
        }
    }
    return hRc;
}

HANDLE WINAPI OpenPlugin(
  int OpenFrom,
  INT_PTR Item
)
{
    HANDLE hRc = INVALID_HANDLE_VALUE;
    if (/*!gbOldConEmu &&*/ hPicViewDll && fOpenPlugin) {
        if (gsTermMsg[0] && psi.Message)
            psi.Message(psi.ModuleNumber, FMSG_MB_OK|FMSG_ALLINONE, NULL, 
                (const TCHAR *const *)gsTermMsg, 0, 0);
        else
            hRc = fOpenPlugin(OpenFrom, Item);
    }
    return hRc;
}

int WINAPI ProcessEditorEvent(
  int Event,
  void *Param
)
{
    if (/*!gbOldConEmu &&*/ hPicViewDll && fProcessEditorEvent)
        return fProcessEditorEvent(Event, Param);
    return FALSE;
}

int WINAPI ProcessViewerEvent(
  int Event,
  void *Param
)
{
    if (/*!gbOldConEmu &&*/ hPicViewDll && fProcessViewerEvent)
        return fProcessViewerEvent(Event, Param);
    return FALSE;
}

void WINAPI SetStartupInfo(
  const struct PluginStartupInfo *Info
)
{
    ::psi = *Info;
    ::fsf = *(Info->FSF);
    
    //if (IsOldConEmu) {
    //    psi.Message(psi.ModuleNumber, FMSG_MB_OK|FMSG_ALLINONE, NULL, 
    //        (const TCHAR *const *)"PictureView wrapper\nConEmu old version detected!\nPlease upgrade!", 0, 0);
    //}
    if (gsLoadMsg[0] && psi.Message) {
        psi.Message(psi.ModuleNumber, FMSG_MB_OK|FMSG_ALLINONE, NULL, 
            (const TCHAR *const *)gsLoadMsg, 0, 0);
    }

    fWrapperAdvControl = psi.AdvControl;
    psi.AdvControl = WrapperAdvControl;

    psi.FSF = &fsf;

    if (hPicViewDll && fSetStartupInfo)
        fSetStartupInfo(&psi);
}

INT_PTR WINAPI WrapperAdvControl(int ModuleNumber,int Command,void *Param)
{
    if (!fWrapperAdvControl)
        return 0;

    if (Command==ACTL_GETFARHWND && !IsOldConEmu() && ghConEmu) {
        HWND hFarWnd = (HWND)fWrapperAdvControl(ModuleNumber, ACTL_GETFARHWND, NULL);
        //if (IsWindowVisible(hFarWnd))
        //    return (INT_PTR)hFarWnd;
            
        if (IsWindow(ghConEmu)) {
            return (INT_PTR)ghConEmu;
        } else {
            return (INT_PTR)hFarWnd;
        }
    }

    return fWrapperAdvControl(ModuleNumber, Command, Param);
}

//HWND AtoH(TCHAR *Str, int Len)
//{
//    DWORD Ret=0;
//    for (; Len && *Str; Len--, Str++)
//    {
//        if (*Str>=_T('0') && *Str<=_T('9'))
//            (Ret*=16)+=*Str-_T('0');
//        else if (*Str>=_T('a') && *Str<=_T('f'))
//            (Ret*=16)+=(*Str-_T('a')+10);
//        else if (*Str>=_T('A') && *Str<=_T('F'))
//            (Ret*=16)+=(*Str-_T('A')+10);
//    }
//    return (HWND)Ret;
//}

BOOL IsOldConEmu()
{
    if (!gbConEmuChecked) {
	    int nRc = -1;
	    
	    gbConEmuChecked = TRUE;
	    
	    ghConEmu = NULL;
	    nRc = ConEmuCheck(&ghConEmu);
		// 0 -- All OK (ConEmu found, Version OK)
		// 1 -- NO ConEmu (simple console mode)
		// 2 -- ConEmu found, but old version
	    if (ghConEmu && nRc==2)
		    gbOldConEmu = TRUE;
    }

    if (gbOldConEmu && !gbOldConEmuWarned && psi.Message)
    {
	    gbOldConEmuWarned = TRUE;
        psi.Message(psi.ModuleNumber, FMSG_MB_OK|FMSG_ALLINONE, NULL, 
            (const TCHAR *const *)"PictureView wrapper\nConEmu old version detected!\nPlease upgrade!", 0, 0);
    }
    return gbOldConEmu;
}