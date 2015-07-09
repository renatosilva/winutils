/*
 * SendTray
 * Copyright (c) 1998-2011 Original RBTray authors
 *               2014 Renato Silva
 * GNU GPLv2 licensed
 *
 */
#include <windows.h>
#include "sendtray.h"

static HHOOK _hMouse = NULL;
static HHOOK _hWndProc = NULL;
static HWND _hLastHit = NULL;

// Works for 32-bit and 64-bit apps.

LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode >= 0) {
		if (wParam == WM_NCRBUTTONDOWN || wParam == WM_NCRBUTTONUP) {
			MOUSEHOOKSTRUCT *info = (MOUSEHOOKSTRUCT*)lParam;
			BOOL isHit = (info->wHitTestCode == HTMINBUTTON);
			if (wParam == WM_NCRBUTTONDOWN && isHit) {
				_hLastHit = info->hwnd;
				return 1;
			}
			else if (wParam == WM_NCRBUTTONUP && isHit) {
				if (info->hwnd == _hLastHit) {
					PostMessage(FindWindow(HOOK_NAME, HOOK_NAME), WM_ADDTRAY, 0, (LPARAM)info->hwnd);
				}
				_hLastHit = NULL;
				return 1;
			}
			else {
				_hLastHit = NULL;
			}
		}
		else if (wParam == WM_RBUTTONDOWN || wParam == WM_RBUTTONUP) {
			_hLastHit = NULL;
		}
	}
	return CallNextHookEx(_hMouse, nCode, wParam, lParam);
}

// Only works for 32-bit apps or 64-bit apps depending on whether this is
// complied as 32-bit or 64-bit.

LRESULT CALLBACK CallWndRetProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode >= 0) {
		CWPRETSTRUCT *msg = (CWPRETSTRUCT*)lParam;
		if ((msg->message == WM_WINDOWPOSCHANGED &&
			 (((WINDOWPOS*)msg->lParam)->flags & SWP_SHOWWINDOW) != 0) ||
			(msg->message == WM_NCDESTROY))
		{
			PostMessage(FindWindow(HOOK_NAME, HOOK_NAME), WM_REFRTRAY, 0, (LPARAM)msg->hwnd);
		}
	}
	return CallNextHookEx(_hWndProc, nCode, wParam, lParam);
}

BOOL DLLIMPORT RegisterHook(HMODULE hLib) {
	_hMouse = SetWindowsHookEx(WH_MOUSE, (HOOKPROC)MouseProc, hLib, 0);
	_hWndProc = SetWindowsHookEx(WH_CALLWNDPROCRET, (HOOKPROC)CallWndRetProc, hLib, 0);
	if (_hMouse == NULL || _hWndProc == NULL) {
		UnRegisterHook();
		return FALSE;
	}
	return TRUE;
}

void DLLIMPORT UnRegisterHook() {
	if (_hMouse) {
		UnhookWindowsHookEx(_hMouse);
		_hMouse = NULL;
	}
	if (_hWndProc) {
		UnhookWindowsHookEx(_hWndProc);
		_hWndProc = NULL;
	}
}
