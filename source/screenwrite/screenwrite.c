#define APP_NAME         "ScreenWrite"
#define APP_COPYRIGHT    "Copyright (c) 2015 Renato Silva"
#define APP_LICENSE      "GNU GPLv2 licensed"

#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <windows.h>
#include <commctrl.h>
#include "version.h"

#define DELAY_TIMER 1
#define CURSOR_TIMER 2
#define CENTERED LONG_MIN
#define NO_COLOR LONG_MIN
#define RGB_COLOR(integer) RGB(integer >> 16, integer >> 8, integer)
#define TRANSPARENT_COLOR(color) (color == 0x000000)
#define BACKGROUND_COLOR(color) (color == NO_COLOR? TRANSPARENT_COLOR(color) : RGB_COLOR(color))

#define APP_HELP APP_NAME " " APP_VERSION "\n" APP_COPYRIGHT "\n" APP_LICENSE "\n\n" \
                 "This program prints standard input to the screen. " \
                 "Output from other programs can be redirected, and screen will keep displaying the current line. " \
                 "Lines beginning with [header] are split in two before printing.\n\n" \
                 "Usage: screenwrite [font=NAME] [size=NUMBER] [color=0xRGB] [background=0xRGB] [weight=NUMBER] [italic=1] " \
                 "[top=POSITION] [left=POSITION] [delay=MILLISECONDS] [cursor=MILLISECONDS] [taskbar=1] [antialiasing=0] [initial text]"

#define FAILURE_HELP                 100
#define FAILURE_CANCELED             101
#define FAILURE_REGISTERING_CLASS    102
#define FAILURE_CREATING_WINDOW      103
#define FAILURE_CHECKING_INPUT       104
#define FAILURE_READING_INPUT        105

static HFONT font;
static WNDCLASS class;
static POINT cursor_position;
static ssize_t read_result;
static BOOL sleeping = FALSE;
static BOOL taskbar = FALSE;
static BOOL antialiasing = TRUE;
static BOOL font_italic = FALSE;
static char *current_line = NULL;
static char *font_name = "Segoe UI Semilight";
static int font_weight = FW_NORMAL;
static int background = NO_COLOR;
static int font_color = 0xd8d8d8;
static int font_size = 36;
static int left = CENTERED;
static int top = CENTERED;
static int cursor = 1000;
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

	if (!line)
		return;
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
	free(current_line);
	current_line = NULL;
}

static void update_window(HWND window) {
	UpdateWindow(window);
	if (!delay)
		return;
	SetTimer(window, DELAY_TIMER, delay, NULL);
	sleeping = TRUE;
}

LRESULT CALLBACK WindowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
	int window_width;
	int window_height;
	BOOL mouse_moved;
	BOOL show_cursor;
	HGDIOBJ replaced;
	HBITMAP bitmap;
	PAINTSTRUCT paint;
	RECT rectangle;
	HDC context;

	switch (message) {
		case WM_PAINT:
			BeginPaint(window, &paint);
			context = CreateCompatibleDC(paint.hdc);
			GetWindowRect(window, &rectangle);
			window_width = rectangle.right - rectangle.left;
			window_height = rectangle.bottom - rectangle.top;
			bitmap = CreateCompatibleBitmap(paint.hdc, window_width, window_height);
			replaced = SelectObject(context, bitmap);

			FillRect(context, &rectangle, class.hbrBackground);
			SetBkColor(context, BACKGROUND_COLOR(background));
			SetBkMode(context, OPAQUE);
			SelectObject(context, font);
			SetTextColor(context, RGB_COLOR(font_color));
			draw_text(context, rectangle);

			BitBlt(paint.hdc, 0, 0, window_width, window_height, context, 0, 0, SRCCOPY);
			SelectObject(context, replaced);
			DeleteObject(bitmap);
			DeleteDC(context);
			EndPaint(window, &paint);
			break;
		case WM_TIMER:
			switch (wParam) {
				case DELAY_TIMER: sleeping = FALSE; break;
				case CURSOR_TIMER: SetCursor(NULL); break;
			}
			break;
		case WM_MOUSEMOVE:
			mouse_moved = cursor_position.x != MAKEPOINTS(lParam).x ||
			              cursor_position.y != MAKEPOINTS(lParam).y;
			show_cursor = cursor < 0 || (cursor && mouse_moved);
			if (show_cursor && cursor > 0)
				SetTimer(window, CURSOR_TIMER, cursor, NULL);
			SetCursor(show_cursor? LoadCursor(NULL, IDC_ARROW) : NULL);
			cursor_position.x = MAKEPOINTS(lParam).x;
			cursor_position.y = MAKEPOINTS(lParam).y;
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
			!get_number_option(__argv[index], "cursor",       &cursor)       &&
			!get_number_option(__argv[index], "delay",        &delay)        &&
			!get_number_option(__argv[index], "top",          &top)          &&
			!get_number_option(__argv[index], "left",         &left)         &&
			!get_number_option(__argv[index], "taskbar",      &taskbar)      &&
			!get_number_option(__argv[index], "background",   &background)   &&
			!get_number_option(__argv[index], "antialiasing", &antialiasing) &&
			!current_line)
			asprintf(&current_line, "%s", __argv[index]);
	}

	class.hInstance     = instance;
	class.lpszClassName = APP_NAME;
	class.lpfnWndProc   = &WindowProcedure;
	class.hbrBackground = CreateSolidBrush(BACKGROUND_COLOR(background));
	class.hIcon         = (HICON) LoadImage(instance, MAKEINTRESOURCE(0), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_DEFAULTCOLOR | LR_SHARED);
	class.hCursor       = NULL;
	class.style         = CS_HREDRAW | CS_VREDRAW;
	class.lpszMenuName  = NULL;
	class.cbClsExtra    = 0;
	class.cbWndExtra    = 0;

	if (!RegisterClass(&class))
		return FAILURE_REGISTERING_CLASS;
	if (!(window = CreateWindowEx(WS_EX_LAYERED | (taskbar? 0 : WS_EX_TOOLWINDOW), APP_NAME, APP_NAME, WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, NULL, NULL, instance, NULL)))
		return FAILURE_CREATING_WINDOW;

	font = CreateFont(font_size, 0, 0, 0, font_weight, font_italic, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, antialiasing? CLEARTYPE_QUALITY : NONANTIALIASED_QUALITY, DEFAULT_PITCH, font_name);
	SetLayeredWindowAttributes(window, TRANSPARENT_COLOR(background), 0, LWA_COLORKEY);
	ShowWindow(window, SW_MAXIMIZE);
	GetCursorPos(&cursor_position);
	if (current_line)
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

		while (TRUE) {
			char character;
			read_result = read(STDIN_FILENO, &character, 1);
			if (read_result < 0)
				return FAILURE_READING_INPUT;
			if (read_result == 0)
				return EXIT_SUCCESS;
			if (character == '\n' || character == '\r')
				break;
			char *old_current_line = current_line;
			asprintf(&current_line, "%s%c", current_line? current_line : "", character);
			free(old_current_line);
		}
		GetWindowRect(window, &rectangle);
		InvalidateRect(window, &rectangle, FALSE);
		update_window(window);
	}
	return message.wParam;
}
