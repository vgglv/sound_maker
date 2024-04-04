#include <cstddef>
#include <string.h>
#include <windows.h>
#include <mmsystem.h>
#include <cmath>
#include <assert.h>
#include <string>
#include <winuser.h>
#include <tchar.h>

namespace {
	UINT timer_id;
	HWND button_hwnd;
	HWND main_window_hwnd;
	HWND edittext_hwnd;
	bool turned_on = false;
}

#define IDC_BUTTON 1234
NOTIFYICONDATA nid;
bool createTimer();
LRESULT CALLBACK EditProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    char greeting[] = "";
	switch (message) {
	case WM_CLOSE:
	{
		ShowWindow(main_window_hwnd, SW_HIDE);
		return 0;
	}
	case WM_USER + 1:
	{
		switch (lParam) {
			case WM_LBUTTONDOWN:
			{
				ShowWindow(main_window_hwnd, SW_SHOW);
				break;
			}
		}
		return 0;
	}
	case WM_CREATE: 
		break;
	case WM_COMMAND: 
	{
		switch (LOWORD(wParam)) {
		case IDC_BUTTON: 
		{
			if (timer_id != 0) {
				KillTimer(NULL, timer_id);
			}
			if (!turned_on) {
				if (!createTimer()) {
					break;
				}
			}
			turned_on = !turned_on;
			std::string button_name;
			if (turned_on) {
				button_name = "OFF";
			} else {
				button_name = "ON";
			}
			SendMessage(button_hwnd, WM_SETTEXT, 0, (LPARAM)button_name.data());
			break;
		}
		default:
			break;
		}
	}
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);

		// All painting occurs here, between BeginPaint and EndPaint.

		FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
		RECT rc;

		HDC dc = GetDC(hwnd);
		SetRect(&rc, 5, 5, 395, 200);
		const char* lpzs = "This application will send silent audio, so that your bluetooth headphones wouldn't turn off\n\nTimer (ms):";
		DrawText(dc, lpzs, strlen(lpzs), &rc, DT_LEFT | DT_EXTERNALLEADING | DT_WORDBREAK);
		ReleaseDC(hwnd, dc);

		EndPaint(hwnd, &ps);
		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		break;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}

void playSound() {
	DWORD duration = 1; // 1000 ms = 1 second

	WAVEFORMATEX format;
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = 1;		   // Mono
	format.nSamplesPerSec = 44100; // Sample rate (Hz)
	format.nAvgBytesPerSec = format.nSamplesPerSec * sizeof(WORD);
	format.nBlockAlign = sizeof(WORD);
	format.wBitsPerSample = sizeof(WORD) * 8;
	format.cbSize = 0;

	// Open the waveform-audio output device for playback
	HWAVEOUT h_wave_out;
	if (waveOutOpen(&h_wave_out, WAVE_MAPPER, &format, 0, 0, CALLBACK_NULL) == MMSYSERR_NOERROR)
	{
		// Generate the waveform data
		int sampleRate = format.nSamplesPerSec;
		int samples = sampleRate * (duration / 1000); // Calculate number of samples
		WORD *data = new WORD[samples]();

		// Prepare and send the waveform-audio data for playback
		WAVEHDR wave_hdr;
		wave_hdr.lpData = (LPSTR)data;
		wave_hdr.dwBufferLength = samples * sizeof(WORD);
		wave_hdr.dwBytesRecorded = 0;
		wave_hdr.dwUser = 0;
		wave_hdr.dwFlags = 0;
		wave_hdr.dwLoops = 0;
		waveOutPrepareHeader(h_wave_out, &wave_hdr, sizeof(WAVEHDR));
		waveOutWrite(h_wave_out, &wave_hdr, sizeof(WAVEHDR));

		// Wait for sound to finish playing (optional)
		Sleep(duration);

		// Clean up resources
		delete[] data;
		waveOutUnprepareHeader(h_wave_out, &wave_hdr, sizeof(WAVEHDR));
		waveOutClose(h_wave_out);
	}
}

VOID CALLBACK TimerCallback(HWND uTimerID, UINT uMsg, UINT_PTR dwUser, DWORD dwParam) {
	playSound();
}

bool createTimer() {
	TCHAR buffer[256];
	GetWindowText(edittext_hwnd, buffer, 256);

	int time = atoi(buffer);
	if (time <= 0) {
		MessageBox(NULL, TEXT("Failed to set timer"), TEXT("Error"), MB_ICONERROR);
		return false;
	}
	timer_id = SetTimer(NULL, 0, time, TimerCallback);

	if (timer_id == 0) {
		// Error handling
		MessageBox(NULL, TEXT("Failed to set timer"), TEXT("Error"), MB_ICONERROR);
		return false;
	}
	return true;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow) {
	std::string class_name = "Main_window";

	WNDCLASS wc = {};

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = class_name.data();

	RegisterClass(&wc);

	// Create the window.

	main_window_hwnd = CreateWindowEx(
		0,							 // Optional window styles.
		class_name.data(),					 // Window class
		"Sound maker", // Window text
		WS_OVERLAPPEDWINDOW,		 // Window style

		// Size and position
		CW_USEDEFAULT, CW_USEDEFAULT, 400, 300,

		NULL,	   // Parent window
		NULL,	   // Menu
		hInstance, // Instance handle
		NULL	   // Additional application data
	);

	button_hwnd = CreateWindow( 
		"BUTTON",  // Predefined class; Unicode assumed 
		"On",      // Button text 
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
		5,         // x position 
		90,         // y position 
		100,        // Button width
		20,        // Button height
		main_window_hwnd,     // Parent window
		(HMENU)IDC_BUTTON,       // No menu.
		(HINSTANCE)GetWindowLongPtr(main_window_hwnd, GWLP_HINSTANCE), 
		NULL // Pointer not needed.
	);

	edittext_hwnd = CreateWindowEx(
		0, "EDIT", "10000", 
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL,
		5, 70, 250, 20,
		main_window_hwnd, NULL, hInstance, NULL
	);
	SetWindowLongPtr(edittext_hwnd, GWLP_USERDATA, (LONG_PTR)EditProc);

	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = main_window_hwnd;
	nid.uID = 1;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.uCallbackMessage = WM_USER + 1;
	nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	strcpy_s(nid.szTip, "Sound master");
	Shell_NotifyIcon(NIM_ADD, &nid);

	assert(main_window_hwnd != NULL && "hwnd is null");

	ShowWindow(main_window_hwnd, nCmdShow);

	MSG msg = {};
	while (GetMessage(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	if (timer_id != 0) {
		KillTimer(NULL, timer_id);
	}
	Shell_NotifyIcon(NIM_DELETE, &nid);
	//int result = PlaySoundEveryFiveMinutes();
	//return result;
	return 0;
}

LRESULT CALLBACK EditProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_CHAR:
			// Allow only numeric characters
			if (wParam < '0' || wParam > '9')
				return 0;
			break;
	}

	// Call the default window procedure for other messages
	return CallWindowProc((WNDPROC)GetWindowLongPtr(hwnd, GWLP_USERDATA), hwnd, uMsg, wParam, lParam);
}