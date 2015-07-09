/*
 * Toggle Hidden Files 2014.6.3
 * Copyright (c) 2014 Renato Silva
 * GNU GPLv2 licensed
 *
 */
#include <shlobj.h>

static HANDLE cabinets[1024];
static int lastCabinet = -1;

BOOL IsClass(HWND handle, const char* targetClass) {
	TCHAR class[256];
	GetClassName(handle, class, sizeof(class) / sizeof(TCHAR));
	return strcmp(class, targetClass) == 0;
}

BOOL FindCabinets(HWND window, LPARAM param) {
	if (IsClass(window, "CabinetWClass"))
		cabinets[++lastCabinet] = window;
	return TRUE;
}

static BOOL UpdateContentArea(HWND handle, LPARAM lParam) {
	if (IsClass(handle, "DirectUIHWND"))
		PostMessage(handle, WM_KEYDOWN, VK_F5, 0);
	return TRUE;
}

static BOOL UpdateExplorerWindows() {
	EnumWindows((WNDENUMPROC) FindCabinets, 0);
	for (int i = 0; i <= lastCabinet; i++)
		EnumChildWindows(cabinets[i], (WNDENUMPROC) UpdateContentArea, 0);
}

void ToggleHiddenFiles() {
	SHELLSTATE state;
	SHGetSetSettings(&state, SSF_SHOWALLOBJECTS, FALSE);
	state.fShowAllObjects = !state.fShowAllObjects;
	SHGetSetSettings(&state, SSF_SHOWALLOBJECTS, TRUE);
	UpdateExplorerWindows();
}

LRESULT LowLevelKeyboardProcedure(int code, WPARAM wParam, LPARAM lParam) {
	if (code >= 0 &&
		GetAsyncKeyState(VK_CONTROL) &&
		wParam == WM_KEYDOWN && ((PKBDLLHOOKSTRUCT) lParam)->vkCode == 'H' &&
		IsClass(GetForegroundWindow(), "CabinetWClass"))
			ToggleHiddenFiles();
	return CallNextHookEx(NULL, code, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous, LPSTR command, int show) {
	MSG message;
	HHOOK hook;
	if ((hook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC) LowLevelKeyboardProcedure, NULL, 0)) == NULL)
		return EXIT_FAILURE;
	while(GetMessage(&message, NULL, 0, 0) > 0) {
		TranslateMessage(&message);
		DispatchMessage(&message);
	}
	UnhookWindowsHookEx(hook);
	return EXIT_SUCCESS;
}
