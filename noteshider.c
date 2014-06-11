/*
 * Sticky Notes Taskbar Icon Hider 2014.6.11
 * Copyright (c) 2014 Renato Silva
 * GNU GPLv2 licensed
 *
 */
#include <windows.h>

BOOL IsClass(HWND handle, const char* targetClass) {
	TCHAR class[256];
	GetClassName(handle, class, sizeof(class) / sizeof(TCHAR));
	return strcmp(class, targetClass) == 0;
}

BOOL HideStickyNotes(HWND window, LPARAM param) {
	if (IsClass(window, "Sticky_Notes_Top_Window"))
		ShowWindow(window, SW_HIDE);
	return TRUE;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous, LPSTR command, int show) {
	MSG message = {0};
	HHOOK hook;
	while (message.message != WM_QUIT) {
		while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
		EnumWindows((WNDENUMPROC) HideStickyNotes, 0);
		Sleep(1000);
	}
	return EXIT_SUCCESS;
}
