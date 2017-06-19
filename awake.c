#include <windows.h>

#define WM_USER_SHELLICON (WM_USER + 1)
#define IDM_ABOUT 100
#define IDM_EXIT 101

HICON hicon_awake, hicon_normal;

/* command=1: add icon. defaults to keep awake
   command=2: toggle status between "keep awake" and normal.
   command=3: remove icon */
void UpdateStatus (int command, HWND hwnd)
{
	static int status_awake = 1; // 0: normal, 1: keep awake.

	if (command == 2)
		status_awake = !status_awake;

	if (status_awake) {
		SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED);
	} else {
		SetThreadExecutionState(ES_CONTINUOUS);
	}

	NOTIFYICONDATA notify;
	ZeroMemory(&notify, sizeof(notify));
	notify.cbSize = sizeof(notify);
	notify.hWnd = hwnd;
	notify.uID = 1;
	notify.uFlags = NIF_ICON | NIF_MESSAGE;
	notify.hIcon = status_awake ? hicon_awake : hicon_normal;
	notify.uCallbackMessage = WM_USER_SHELLICON;

	DWORD op = (DWORD)-1;
	switch (command) {
		case 1: op = NIM_ADD; break;
		case 2: op = NIM_MODIFY; break;
		case 3: op = NIM_DELETE; break;
	}
	if (!Shell_NotifyIcon(op, &notify)) {
		MessageBox(NULL, "Shell_NotifyIcon failed.", "Error", MB_OK | MB_ICONERROR);
	}
}

void ShowMenu (HWND hwnd)
{
	POINT point;
	GetCursorPos(&point);

	HMENU hmenu = CreatePopupMenu();
	InsertMenu(hmenu, 0, MF_BYPOSITION|MF_STRING, IDM_ABOUT, "About");
	InsertMenu(hmenu, 0, MF_BYPOSITION|MF_STRING, IDM_EXIT, "Exit");
	TrackPopupMenu(hmenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON, point.x, point.y, 0, hwnd, NULL);
	DestroyMenu(hmenu);
}

LRESULT CALLBACK WindowProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
		case WM_USER_SHELLICON:
			switch (LOWORD(lParam)) {
				case WM_LBUTTONDOWN:
					UpdateStatus(2, hwnd);
					return 0;
				case WM_RBUTTONDOWN:
					ShowMenu(hwnd);
					return 0;
				default:
					return DefWindowProc(hwnd, uMsg, wParam, lParam);
			}
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDM_ABOUT:
					MessageBox(hwnd, "Keep computer from sleep, screen saver and lock.", "About", MB_OK | MB_ICONINFORMATION);
					return 0;
				case IDM_EXIT:
					DestroyWindow(hwnd);
					return 0;
			}
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		default:
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}

int CALLBACK WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
	const char *CLASS_NAME = "DUMMY_CLASS";
	WNDCLASSEX wx;
	ZeroMemory(&wx, sizeof(wx));
	wx.cbSize = sizeof(WNDCLASSEX);
	wx.lpfnWndProc = WindowProc;
	wx.hInstance = hInstance;
	wx.lpszClassName = CLASS_NAME;
	if (!RegisterClassEx(&wx)) {
		MessageBox(NULL, "RegisterClassEx failed.", "Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	hicon_awake = LoadIcon(hInstance, MAKEINTRESOURCE(201));
	hicon_normal = LoadIcon(hInstance, MAKEINTRESOURCE(202));

	HWND hwnd = CreateWindowEx(0, CLASS_NAME, "dummy_name", 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL );
	if (hwnd == NULL) {
		MessageBox(NULL, "CreateWindowEx failed.", "Error", MB_OK | MB_ICONERROR);
		return 1;
	}
	ShowWindow(hwnd, SW_HIDE);

	UpdateStatus(1, hwnd);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	UpdateStatus(3, hwnd);
	return 0;
}
