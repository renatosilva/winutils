/*
 * WinUtils Uninstaller Helper
 * Copyright (C) 2015 Renato Silva
 */

#include <windows.h>
#include <tlhelp32.h>

BOOL terminate(const char *name) {
	BOOL result = TRUE;
	HANDLE process;
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(entry);
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
	BOOL has_next = Process32First(snapshot, &entry);

	while (has_next) {
		has_next = Process32Next(snapshot, &entry);
		if (strcmp(entry.szExeFile, name))
			continue;
		if (process = OpenProcess(PROCESS_TERMINATE, FALSE, entry.th32ProcessID)) {
			if (!TerminateProcess(process, 1) && result)
				result = FALSE;
			CloseHandle(process);
		}
	}
	CloseHandle(snapshot);
	return result;
}

DWORD execute(char *command) {
	DWORD result = 1;
	PROCESS_INFORMATION process;
	STARTUPINFO startup = { sizeof(startup) };

	if (CreateProcess(NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &startup, &process)) {
		WaitForSingleObject(process.hProcess, INFINITE);
		GetExitCodeProcess(process.hProcess, &result);
		CloseHandle(process.hProcess);
		CloseHandle(process.hThread);
	}
	return result;
}

int main(int argc, char **argv) {
	if (!terminate("noteshider.exe") ||
		!terminate("screenwrite.exe") ||
		!terminate("togglehidden.exe") ||
		!terminate("webshutdown.exe") ||
		execute("sendtray.exe --exit") != 0)
		return EXIT_FAILURE;
	return execute("do_uninstall.exe");
}
