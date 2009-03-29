#pragma once

class CConEmuChild
{
public:
	CConEmuChild();
	~CConEmuChild();

	LRESULT CALLBACK ChildWndProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
public:
	LRESULT OnPaint(WPARAM wParam, LPARAM lParam);
	LRESULT OnSize(WPARAM wParam, LPARAM lParam);
	HWND Create();

protected:
	UINT mn_MsgTabChanged;
};

class CConEmuBack
{
public:
	CConEmuBack();
	~CConEmuBack();

	HWND mh_Wnd;
	HBRUSH mh_BackBrush;
	COLORREF mn_LastColor;

	HWND Create();
	void Resize();
	void Refresh();

	static LRESULT CALLBACK BackWndProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
};