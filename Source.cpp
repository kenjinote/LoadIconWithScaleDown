#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#pragma comment(lib, "shlwapi")

#include <windows.h>
#include <shlwapi.h>
#include <initializer_list>

TCHAR szClassName[] = TEXT("Window");

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
		DragAcceptFiles(hWnd, TRUE);
		break;
	case WM_DROPFILES:
		{
			TCHAR szFilePath[MAX_PATH];
			const UINT iFileNum = DragQueryFile((HDROP)wParam, -1, NULL, 0);
			if (iFileNum == 1)
			{
				DragQueryFile((HDROP)wParam, 0, szFilePath, MAX_PATH);
				if (PathMatchSpec(szFilePath, TEXT("*.ico")))
				{
					InvalidateRect(hWnd, 0, 1);
					UpdateWindow(hWnd);
					typedef HRESULT(__stdcall * fnLoadIconWithScaleDown)(HINSTANCE, PCWSTR, int, int, HICON *);
					fnLoadIconWithScaleDown LoadIconWithScaleDown;
					HMODULE hModule = LoadLibrary(TEXT("comctl32.dll"));
					LoadIconWithScaleDown = (fnLoadIconWithScaleDown)GetProcAddress(hModule, "LoadIconWithScaleDown");
					int nTop = 0;
					for (auto width : {16, 32, 48, 64, 96, 128, 256})
					{
						HICON hIcon = 0;
						if (LoadIconWithScaleDown(0, szFilePath, width, width, &hIcon) == S_OK ||  // ←なぜか 32x32 の読み込みで失敗する？
							(hIcon = (HICON)LoadImage(0, szFilePath, IMAGE_ICON, width, width, LR_LOADFROMFILE)) != NULL)
						{
							ICONINFO IconInfo;
							GetIconInfo(hIcon, &IconInfo);
							BITMAP bitmap;
							GetObject(IconInfo.hbmColor, sizeof(BITMAP), &bitmap);
							{
								HDC hdc = GetDC(hWnd);
								DrawIconEx(hdc, 0, nTop, hIcon, bitmap.bmWidth, bitmap.bmHeight, 0, 0, DI_NORMAL);
								TCHAR szText[1024];
								wsprintf(szText, TEXT("%dx%d"), bitmap.bmWidth, bitmap.bmHeight);
								TextOut(hdc, bitmap.bmWidth, nTop, szText, lstrlen(szText));
								ReleaseDC(hWnd, hdc);
								nTop += bitmap.bmHeight;
							}
							DestroyIcon(hIcon);
						}
					}
				}
			}
			DragFinish((HDROP)wParam);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInst, LPSTR pCmdLine, int nCmdShow)
{
	MSG msg;
	WNDCLASS wndclass = {
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,
		0,
		hInstance,
		0,
		LoadCursor(0,IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW + 1),
		0,
		szClassName
	};
	RegisterClass(&wndclass);
	HWND hWnd = CreateWindow(
		szClassName,
		TEXT("ドロップされたアイコンファイルを表示"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		CW_USEDEFAULT,
		0,
		CW_USEDEFAULT,
		0,
		0,
		0,
		hInstance,
		0
	);
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}
