/*
 * Copyright (C) 2015, 2016 Renato Silva
 * Licensed under GNU GPLv2
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <libgen.h>
#include <mongoose.h>
#include <windows.h>
#include "version.h"

static char *expected_query = NULL;

static BOOL logoff_and_shutdown() {
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

static void handle_request(struct mg_connection *connection, int event, void *data) {
	if (event == MG_EV_HTTP_REQUEST) {
		struct http_message *message = (struct http_message*) data;
		char *response = "hi";
		if (strstr(message->uri.p, "/shutdown?auth=")) {
			if (mg_vcmp(&message->query_string, expected_query) == 0) {
				logoff_and_shutdown();
				response = "started";
			} else {
				response = "denied";
			}
		}
		mg_send_head(connection, 200, strlen(response), NULL);
		mg_printf(connection, response);
		connection->flags |= MG_F_SEND_AND_CLOSE;
	}
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

	struct mg_mgr manager;
	mg_mgr_init(&manager, NULL);
	struct mg_connection *connection = mg_bind(&manager, argv[1], handle_request);
	mg_set_protocol_http_websocket(connection);
	for (;;) mg_mgr_poll(&manager, 1000);
	mg_mgr_free(&manager);
	return EXIT_SUCCESS;
}
