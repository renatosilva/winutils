/*
 * Sticky Notes Icon Hider
 * Copyright (c) 2014 Renato Silva
 * GNU GPLv2 licensed
 *
 */
#include <windows.h>
#include <stdbool.h>
#include <commctrl.h>

static bool watch = false;
static bool hiddenFromTaskbar = false;
static bool hiddenFromTaskManager = false;
static int noteWindowCount = 0;
static int noteWindowsHidden = 0;

BOOL IsClass(HWND handle, const char* targetClass) {
	TCHAR class[256];
	GetClassName(handle, class, sizeof(class) / sizeof(TCHAR));
	return strcmp(class, targetClass) == 0;
}

BOOL NoteWindowCount(HWND window, LPARAM param) {
	if (IsClass(window, "Sticky_Notes_Note_Window"))
		noteWindowCount++;
	return TRUE;
}

BOOL HideStickyNotes(HWND window, LPARAM param) {

	// Hide from taskbar
	if (IsClass(window, "Sticky_Notes_Top_Window")) {
		ShowWindow(window, SW_HIDE);
		hiddenFromTaskbar = true;
		if (hiddenFromTaskManager)
			return FALSE;
	}

	// Hide from application list in task manager
	else if (IsClass(window, "Sticky_Notes_Note_Window")) {
		SetWindowText(window, NULL);
		noteWindowsHidden++;
		if (noteWindowsHidden == noteWindowCount) {
			hiddenFromTaskManager = true;
			if (hiddenFromTaskbar)
				return FALSE;
		}
	}

	return TRUE;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous, LPSTR command, int show) {
	INITCOMMONCONTROLSEX icc;
	MSG message = {0};
	HHOOK hook;
	bool watch;

	icc.dwSize = sizeof(icc);
	icc.dwICC = ICC_STANDARD_CLASSES;
	InitCommonControlsEx(&icc);

	watch = strcmp(command, "--watch") == 0;
	if (command[0] != '\0' && !watch) {
		MessageBox(NULL, "Usage: noteshider [options]\n\nWithout options, exit after hiding. "
		                 "With --watch, keep running.", "Notes Hider", MB_OK | MB_ICONERROR);
		return EXIT_FAILURE;
	}

	while (message.message != WM_QUIT) {
		while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
		EnumWindows((WNDENUMPROC) NoteWindowCount, 0);
		EnumWindows((WNDENUMPROC) HideStickyNotes, 0);
		if (hiddenFromTaskbar && hiddenFromTaskManager) {
			if (!watch)
				return EXIT_SUCCESS;
			hiddenFromTaskbar = false;
			hiddenFromTaskManager = false;
			noteWindowsHidden = 0;
			noteWindowCount = 0;
		}
		Sleep(1000);
	}
	return EXIT_SUCCESS;
}
