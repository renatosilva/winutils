/*
 * Copyright (C) 2015 Renato Silva
 * Licensed under GNU GPLv2
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <libgen.h>
#include <windows.h>
#include <mongoose.h>
#include "version.h"

static char *expected_query = NULL;

BOOL logoff_and_shutdown() {
	HANDLE token_handle;
	TOKEN_PRIVILEGES shutdown_privilege;

	shutdown_privilege.PrivilegeCount = 1;
	shutdown_privilege.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &shutdown_privilege.Privileges[0].Luid);

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &token_handle))
		return FALSE;
	if (!AdjustTokenPrivileges(token_handle, FALSE, &shutdown_privilege, 0, NULL, NULL) || GetLastError() != ERROR_SUCCESS)
		return FALSE;
	return ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE, 0);
}

int handle_request(struct mg_connection *connection, enum mg_event event) {
	if (event == MG_AUTH)
		return MG_TRUE;

	if (event == MG_REQUEST) {
		if (strcmp(connection->uri, "/shutdown") == 0) {
			if (connection->query_string && strcmp(connection->query_string, expected_query) == 0) {
				logoff_and_shutdown();
				mg_printf_data(connection, "started");
			} else {
				mg_printf_data(connection, "denied");
			}
		} else {
			mg_printf_data(connection, "hi");
		}
		return MG_TRUE;
	}
	return MG_FALSE;
}

int main(int argc, char **argv) {
	if (argc != 3 || !strcmp(argv[1], "--help") || !strcmp(argv[1], "-h")) {
		printf("\n\tWeb Shutdown %s\n", APP_VERSION);
		printf("\tCopyright (C) 2015 Renato Silva\n");
		printf("\tLicensed under GNU GPLv2\n\n");
		printf("Usage:\n\t%s PORT PASSWORD_FILE\n\n", basename(argv[0]));
		return EXIT_FAILURE;
	}

	char password[1024] = "";
	FILE *password_file = fopen(argv[2], "r");
	if (password_file && fgets(password, sizeof(password), password_file))
		strtok(password, "\n");
	fclose(password_file);
	if (!*password) {
		printf("Could not read password from file %s.\n", argv[2]);
		return EXIT_FAILURE;
	}
	asprintf(&expected_query, "auth=%s", password);

	struct mg_server *server = mg_create_server(NULL, handle_request);
	mg_set_option(server, "listening_port", argv[1]);
	for (;;) mg_poll_server(server, 1000);
	mg_destroy_server(&server);
	return EXIT_SUCCESS;
}
