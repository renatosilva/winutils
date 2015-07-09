/*
 * SendTray
 * Copyright (c) 1998-2011 Original RBTray authors
 *               2014, 2015 Renato Silva
 * GNU GPLv2 licensed
 *
 */
#include <windows.h>
#include <string.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <libintl.h>
#include <locale.h>
#include "sendtray.h"

#define VERSION "2015.5.21"
#define NAME "SendTray"
#define _(STRING) gettext(STRING)
#define MAXTRAYITEMS 64

static UINT WM_TASKBAR_CREATED;
static HMODULE _hLib;
static HWND _hwndHook;
static HWND _hwndItems[MAXTRAYITEMS];
static HWND _hwndForMenu;

int FindInTray(HWND hwnd) {
	for (int i = 0; i < MAXTRAYITEMS; i++) {
		if (_hwndItems[i] == hwnd) return i;
	}
	return -1;
}

HICON GetWindowIcon(HWND hwnd) {
	HICON icon;
	if (icon = (HICON)SendMessage(hwnd, WM_GETICON, ICON_SMALL, 0)) return icon;
	if (icon = (HICON)SendMessage(hwnd, WM_GETICON, ICON_BIG, 0)) return icon;
	if (icon = (HICON)GetClassLongPtr(hwnd, GCLP_HICONSM)) return icon;
	if (icon = (HICON)GetClassLongPtr(hwnd, GCLP_HICON)) return icon;
	return LoadIcon(NULL, IDI_WINLOGO);
}

static void AddToTray(int i) {
	NOTIFYICONDATA nid;
	ZeroMemory(&nid, sizeof(nid));
	nid.cbSize           = NOTIFYICONDATA_V2_SIZE;
	nid.hWnd             = _hwndHook;
	nid.uID              = (UINT)i;
	nid.uFlags           = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	nid.uCallbackMessage = WM_TRAYCMD;
	nid.hIcon            = GetWindowIcon(_hwndItems[i]);
	GetWindowText(_hwndItems[i], nid.szTip, sizeof(nid.szTip) / sizeof(nid.szTip[0]));
	nid.uVersion         = NOTIFYICON_VERSION;
	Shell_NotifyIcon(NIM_ADD, &nid);
	Shell_NotifyIcon(NIM_SETVERSION, &nid);
}

static void AddWindowToTray(HWND hwnd) {
	int i = FindInTray(NULL);
	if (i == -1) return;
	_hwndItems[i] = hwnd;
	AddToTray(i);
}

static void MinimizeWindowToTray(HWND hwnd) {
	// Don't minimize MDI child windows
	if ((UINT)GetWindowLongPtr(hwnd, GWL_EXSTYLE) & WS_EX_MDICHILD) return;

	// If hwnd is a child window, find parent window (e.g. minimize button in
	// Office 2007 (ribbon interface) is in a child window)
	if ((UINT)GetWindowLongPtr(hwnd, GWL_STYLE) & WS_CHILD) {
		hwnd = GetAncestor(hwnd, GA_ROOT);
	}

	// Add icon to tray if it's not already there
	if (FindInTray(hwnd) == -1) {
		AddWindowToTray(hwnd);
	}

	// Hide window
	ShowWindow(hwnd, SW_HIDE);
}

static void RemoveFromTray(int i) {
	NOTIFYICONDATA nid;
	ZeroMemory(&nid, sizeof(nid));
	nid.cbSize = NOTIFYICONDATA_V2_SIZE;
	nid.hWnd   = _hwndHook;
	nid.uID    = (UINT)i;
	Shell_NotifyIcon(NIM_DELETE, &nid);
}

static void RemoveWindowFromTray(HWND hwnd) {
	int i = FindInTray(hwnd);
	if (i == -1) return;
	RemoveFromTray(i);
	_hwndItems[i] = 0;
}

static void RestoreWindowFromTray(HWND hwnd) {
	ShowWindow(hwnd, SW_SHOW);
	SetForegroundWindow(hwnd);
	RemoveWindowFromTray(hwnd);
}

static void CloseWindowFromTray(HWND hwnd) {
	// Use PostMessage to avoid blocking if the program brings up a dialog on exit.
	// Also, Explorer windows ignore WM_CLOSE messages from SendMessage.
	PostMessage(hwnd, WM_CLOSE, 0, 0);

	Sleep(50);
	if (IsWindow(hwnd)) Sleep(50);

	if (!IsWindow(hwnd)) {
		// Closed successfully
		RemoveWindowFromTray(hwnd);
	}
}

void RefreshWindowInTray(HWND hwnd) {
	int i = FindInTray(hwnd);
	if (i == -1) return;
	if (!IsWindow(hwnd) || IsWindowVisible(hwnd)) {
		RemoveWindowFromTray(hwnd);
	}
	else {
		NOTIFYICONDATA nid;
		ZeroMemory(&nid, sizeof(nid));
		nid.cbSize = NOTIFYICONDATA_V2_SIZE;
		nid.hWnd   = _hwndHook;
		nid.uID    = (UINT)i;
		nid.uFlags = NIF_TIP;
		GetWindowText(hwnd, nid.szTip, sizeof(nid.szTip) / sizeof(nid.szTip[0]));
		Shell_NotifyIcon(NIM_MODIFY, &nid);
	}
}

void ExecuteMenu() {
	HMENU hMenu;
	POINT point;

	hMenu = CreatePopupMenu();
	if (!hMenu) {
		MessageBox(NULL, _("Error creating menu."), NAME, MB_OK | MB_ICONERROR);
		return;
	}
	AppendMenu(hMenu, MF_STRING, IDM_ABOUT,   _("About SendTray"));
	AppendMenu(hMenu, MF_STRING, IDM_EXIT,    _("Exit SendTray"));
	AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
	AppendMenu(hMenu, MF_STRING, IDM_CLOSE,   _("Close window"));
	AppendMenu(hMenu, MF_STRING, IDM_RESTORE, _("Restore window"));

	GetCursorPos(&point);
	SetForegroundWindow(_hwndHook);

	TrackPopupMenu(hMenu, TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_RIGHTALIGN | TPM_BOTTOMALIGN, point.x, point.y, 0, _hwndHook, NULL);

	PostMessage(_hwndHook, WM_USER, 0, 0);
	DestroyMenu(hMenu);
}

void ShowAboutInfo() {
	MSGBOXPARAMS parameters = {0};
	char* header = "SendTray " VERSION "\n\n";
	char* body = _("Based on RBTray (http://rbtray.sourceforge.net)\nCopyright (c) 2014, 2015 Renato Silva, and others\nGNU GPLv2 licensed");
	size_t length = strlen(header) + strlen(body) + 1;
	char text[length];

	snprintf(text, sizeof(text), "%s%s", header, body);
	parameters.cbSize = sizeof(MSGBOXPARAMS);
	parameters.hwndOwner = NULL;
	parameters.hInstance = GetModuleHandle(NULL);
	parameters.lpszText = text;
	parameters.lpszCaption = NAME;
	parameters.dwStyle = MB_USERICON;
	parameters.lpszIcon = MAKEINTRESOURCE(0);
	parameters.dwContextHelpId = (DWORD_PTR) NULL;
	parameters.lpfnMsgBoxCallback = NULL;
	parameters.dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);
	MessageBoxIndirect(&parameters);
}

LRESULT CALLBACK HookWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDM_RESTORE:
					RestoreWindowFromTray(_hwndForMenu);
					break;
				case IDM_CLOSE:
					CloseWindowFromTray(_hwndForMenu);
					break;
				case IDM_ABOUT:
					ShowAboutInfo();
					break;
				case IDM_EXIT:
					SendMessage(_hwndHook, WM_DESTROY, 0, 0);
					break;
			}
			break;
		case WM_ADDTRAY:
			MinimizeWindowToTray((HWND)lParam);
			break;
		case WM_REMTRAY:
			RestoreWindowFromTray((HWND)lParam);
			break;
		case WM_REFRTRAY:
			RefreshWindowInTray((HWND)lParam);
			break;
		case WM_TRAYCMD:
			switch ((UINT)lParam) {
				case NIN_SELECT:
					RestoreWindowFromTray(_hwndItems[wParam]);
					break;
				case WM_CONTEXTMENU:
					_hwndForMenu = _hwndItems[wParam];
					ExecuteMenu();
					break;
				case WM_MOUSEMOVE:
					RefreshWindowInTray(_hwndItems[wParam]);
					break;
			}
			break;
		case WM_DESTROY:
			for (int i = 0; i < MAXTRAYITEMS; i++) {
				if (_hwndItems[i]) {
					RestoreWindowFromTray(_hwndItems[i]);
				}
			}
			UnRegisterHook();
			FreeLibrary(_hLib);
			PostQuitMessage(0);
			break;
	}

	if (msg == WM_TASKBAR_CREATED) {
		for (int i = 0; i < MAXTRAYITEMS; i++) {
			if (_hwndItems[i]) {
				AddToTray(i);
			}
		}
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

TCHAR* GetLocaleDirectory() {
	static TCHAR path[MAX_PATH];
	GetModuleFileName(NULL, path, MAX_PATH);
	PathRemoveFileSpec(path);
	return strncat(path, "\\locale", MAX_PATH);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR szCmdLine, int iCmdShow) {
	INITCOMMONCONTROLSEX icc;
	WNDCLASS wc;
	MSG msg;

	setlocale(LC_ALL, "");
	bindtextdomain("sendtray", GetLocaleDirectory());
	textdomain("sendtray");

	icc.dwSize = sizeof(icc);
	icc.dwICC = ICC_STANDARD_CLASSES;
	InitCommonControlsEx(&icc);

	_hwndHook = FindWindow(HOOK_NAME, HOOK_NAME);
	if (strstr(szCmdLine, "--exit")) {
		if (_hwndHook)
			SendMessage(_hwndHook, WM_CLOSE, 0, 0);
		return 0;
	}
	if (_hwndHook) {
		MessageBox(NULL, _("SendTray is already running."), NAME, MB_OK | MB_ICONINFORMATION);
		return 0;
	}
	if (!(_hLib = LoadLibrary("SendTray.dll"))) {
		MessageBox(NULL, _("Error loading sendtray.dll."), NAME, MB_OK | MB_ICONERROR);
		return 0;
	}
	if (!RegisterHook(_hLib)) {
		MessageBox(NULL, _("Error setting hook procedure."), NAME, MB_OK | MB_ICONERROR);
		return 0;
	}
	wc.style         = 0;
	wc.lpfnWndProc   = HookWndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = NULL;
	wc.hCursor       = NULL;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = HOOK_NAME;
	if (!RegisterClass(&wc)) {
		MessageBox(NULL, _("Error registering window class."), NAME, MB_OK | MB_ICONERROR);
		return 0;
	}
	if (!(_hwndHook = CreateWindow(HOOK_NAME, HOOK_NAME, WS_OVERLAPPED, 0, 0, 0, 0, (HWND)NULL, (HMENU)NULL, (HINSTANCE)hInstance, (LPVOID)NULL))) {
		MessageBox(NULL, _("Error creating window."), NAME, MB_OK | MB_ICONERROR);
		return 0;
	}
	for (int i = 0; i < MAXTRAYITEMS; i++) {
		_hwndItems[i] = NULL;
	}
	WM_TASKBAR_CREATED = RegisterWindowMessage("TaskbarCreated");

	while (IsWindow(_hwndHook) && GetMessage(&msg, _hwndHook, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}
