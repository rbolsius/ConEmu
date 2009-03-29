#pragma once

class CConEmuPipe
{
protected:
   HANDLE hEventCmd[MAXCMDCOUNT];
public:
   HANDLE hEventAlive, hEventReady;
   static HANDLE hMapping;
   static TCHAR  sLastOp[64];
   TCHAR  ms_LastOp[64];
   LPBYTE lpMap, lpCursor;
   DWORD  dwMaxDataSize, nPID;
   WCHAR  sMapName[MAX_PATH];
public:
   CConEmuPipe();
   ~CConEmuPipe();
   void Close();
   BOOL Init(LPCTSTR asOp, BOOL abSilent=FALSE);
   BOOL Execute(int nCmd);
   BOOL Read(LPVOID pData, DWORD nSize, DWORD* nRead);
   LPBYTE GetPtr();
};