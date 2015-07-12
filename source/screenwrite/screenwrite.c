#define APP_NAME         "ScreenWrite"
#define APP_COPYRIGHT    "Copyright (c) 2015 Renato Silva"
#define APP_LICENSE      "GNU GPLv2 licensed"

#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <windows.h>
#include <commctrl.h>
#include "version.h"

#define MAX_LINE 1024
#define CENTERED LONG_MIN
#define APP_HELP APP_NAME " " APP_VERSION "\n" APP_COPYRIGHT "\n" APP_LICENSE "\n\n" \
                 "This program prints standard input to the screen. " \
                 "Output from other programs can be redirected, and screen will keep displaying the current line. " \
                 "Lines beginning with [header] are split in two before printing.\n\n" \
                 "Usage: screenwrite [font=NAME] [size=NUMBER] [color=0xRGB] [weight=NUMBER] [italic=1] [top=POSITION] [left=POSITION] " \
                 "[delay=MILLISECONDS] [taskbar=1] [antialiasing=0] [initial text]"

#define FAILURE_HELP                 100
#define FAILURE_CANCELED             101
#define FAILURE_REGISTERING_CLASS    102
#define FAILURE_CREATING_WINDOW      103
#define FAILURE_CHECKING_INPUT       104
#define FAILURE_READING_INPUT        105

static HFONT font;
static ssize_t read_result;
static BOOL sleeping = FALSE;
static BOOL taskbar = FALSE;
static BOOL antialiasing = TRUE;
static BOOL font_italic = FALSE;
static char current_line[MAX_LINE];
static char *font_name = "Segoe UI Semilight";
static int font_weight = FW_NORMAL;
static int font_color = 0xd8d8d8;
static int font_size = 36;
static int left = CENTERED;
static int top = CENTERED;
static int delay = 0;

static BOOL get_string_option(char *argument, const char *option, char **value) {
	char *format;
	BOOL result;
	asprintf(&format, "%s=", option);
	if (result = strncmp(format, argument, strlen(format)) == 0) {
		while (argument[0] != '=')
			argument++;
		*value = ++argument;
	}
	free(format);
	return result;
}

static BOOL get_number_option(char *argument, const char *option, int *value) {
	char *string;
	if (!get_string_option(argument, option, &string))
		return FALSE;
	*value = strtol(string, NULL, 0);
	return TRUE;
}

static void draw_text(HDC context, RECT rectangle) {
	UINT format = DT_NOPREFIX | DT_WORDBREAK | DT_WORD_ELLIPSIS;
	char *line = current_line;

	if (top == CENTERED) {
		RECT text = rectangle;
		int rectangle_height = rectangle.bottom - rectangle.top;
		int text_height = DrawText(context, current_line, -1, &text, format | DT_CALCRECT);
		rectangle.top = (rectangle_height - text_height) / 2;
	} else {
		rectangle.top = top;
	}
	if (left == CENTERED)
		format |= DT_CENTER;
	else
		rectangle.left = left;
	if (*line == '[') {
		while (*++line) {
			if (*line == ']') {
				*line = '\n';
				line = current_line;
				line++;
				break;
			}
		}
	}
	DrawText(context, line, -1, &rectangle, format | DT_NOCLIP);
}

static void update_window(HWND window) {
	UpdateWindow(window);
	if (!delay)
		return;
	SetTimer(window, 1, delay, NULL);
	sleeping = TRUE;
}

LRESULT CALLBACK WindowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
	PAINTSTRUCT paint;
	RECT rectangle;
	HDC context;

	switch (message) {
		case WM_PAINT:
			BeginPaint(window, &paint);
			context = paint.hdc;
			SetBkColor(context, 0);
			SetBkMode(context, OPAQUE);
			SelectObject(context, font);
			SetTextColor(context, RGB(font_color >> 16, font_color >> 8, font_color));
			GetWindowRect(window, &rectangle);
			draw_text(context, rectangle);
			EndPaint(window, &paint);
			break;
		case WM_TIMER:
			sleeping = FALSE;
			break;
		case WM_DESTROY:
			PostQuitMessage(FAILURE_CANCELED);
			return 0;
	}
	return DefWindowProc(window, message, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous, LPSTR command, int show) {
	static int index;
	INITCOMMONCONTROLSEX controls;
	DWORD has_input;
	RECT rectangle;
	WNDCLASS class;
	HWND window;
	MSG message;

	controls.dwSize = sizeof(controls);
	controls.dwICC = ICC_STANDARD_CLASSES;
	InitCommonControlsEx(&controls);

	for (index = 1; index < __argc; index++) {
		if (strcmp(__argv[index], "--help") == 0 || strcmp(__argv[index], "-h") == 0) {
			MessageBox(NULL, APP_HELP, APP_NAME, MB_ICONINFORMATION | MB_OK);
			return FAILURE_HELP;
		}
		if (!get_string_option(__argv[index], "font",         &font_name)    &&
			!get_number_option(__argv[index], "size",         &font_size)    &&
			!get_number_option(__argv[index], "color",        &font_color)   &&
			!get_number_option(__argv[index], "weight",       &font_weight)  &&
			!get_number_option(__argv[index], "italic",       &font_italic)  &&
			!get_number_option(__argv[index], "delay",        &delay)        &&
			!get_number_option(__argv[index], "top",          &top)          &&
			!get_number_option(__argv[index], "left",         &left)         &&
			!get_number_option(__argv[index], "taskbar",      &taskbar)      &&
			!get_number_option(__argv[index], "antialiasing", &antialiasing) &&
			!current_line[0])
			snprintf(current_line, MAX_LINE, "%s", __argv[index]);
	}

	class.hInstance     = instance;
	class.lpszClassName = APP_NAME;
	class.lpfnWndProc   = &WindowProcedure;
	class.hbrBackground = CreateSolidBrush(0);
	class.hIcon         = (HICON) LoadImage(instance, MAKEINTRESOURCE(0), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_DEFAULTCOLOR | LR_SHARED);
	class.hCursor       = LoadCursor(NULL, IDC_ARROW);
	class.style         = CS_HREDRAW | CS_VREDRAW;
	class.lpszMenuName  = NULL;
	class.cbClsExtra    = 0;
	class.cbWndExtra    = 0;

	if (!RegisterClass(&class))
		return FAILURE_REGISTERING_CLASS;
	if (!(window = CreateWindowEx(WS_EX_LAYERED | (taskbar? 0 : WS_EX_TOOLWINDOW), APP_NAME, APP_NAME, WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, NULL, NULL, instance, NULL)))
		return FAILURE_CREATING_WINDOW;

	font = CreateFont(font_size, 0, 0, 0, font_weight, font_italic, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, antialiasing? CLEARTYPE_QUALITY : NONANTIALIASED_QUALITY, DEFAULT_PITCH, font_name);
	SetLayeredWindowAttributes(window, 0, 0, LWA_COLORKEY);
	ShowWindow(window, SW_MAXIMIZE);
	if (current_line[0])
		update_window(window);

	while (message.message != WM_QUIT) {
		while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
		Sleep(1);
		if (sleeping)
			continue;
		if (!PeekNamedPipe(GetStdHandle(STD_INPUT_HANDLE), NULL, 0, NULL, &has_input, NULL))
			return FAILURE_CHECKING_INPUT;
		if (!has_input)
			continue;

		index = 0;
		while (TRUE) {
			char character;
			read_result = read(STDIN_FILENO, &character, 1);
			if (read_result < 0)
				return FAILURE_READING_INPUT;
			if (read_result == 0)
				return EXIT_SUCCESS;
			if (character == '\n' || character == '\r')
				break;
			current_line[index] = character;
			if (++index == (MAX_LINE - 1))
				break;
		}
		current_line[index] = '\0';
		GetWindowRect(window, &rectangle);
		InvalidateRect(window, &rectangle, TRUE);
		update_window(window);
	}
	return message.wParam;
}
