#include <iostream>
#include <windows.h>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <sstream>
#include <codecvt>

// t800 Comments: Function to convert wstring to string
std::string wstringToString(const std::wstring& wstr) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(wstr);
}

// t800 Comments: Function to capture a screenshot of the specified window
void CaptureScreenshot(HWND hwnd, const std::string& filename) {
    RECT rc;
    GetClientRect(hwnd, &rc);
    int width = rc.right - rc.left;
    int height = rc.bottom - rc.top;

    HDC hdcScreen = GetDC(NULL);
    HDC hdcWindow = GetDC(hwnd);
    HDC hdcMemDC = CreateCompatibleDC(hdcWindow);
    HBITMAP hbmScreen = CreateCompatibleBitmap(hdcWindow, width, height);

    SelectObject(hdcMemDC, hbmScreen);
    BitBlt(hdcMemDC, 0, 0, width, height, hdcWindow, 0, 0, SRCCOPY);
    BITMAP bmpScreen;
    GetObject(hbmScreen, sizeof(BITMAP), &bmpScreen);

    BITMAPFILEHEADER   bmfHeader;
    BITMAPINFOHEADER   bi;

    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bmpScreen.bmWidth;
    bi.biHeight = bmpScreen.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    DWORD dwBmpSize = ((bmpScreen.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmpScreen.bmHeight;

    HANDLE hDIB = GlobalAlloc(GHND, dwBmpSize);
    char* lpbitmap = (char*)GlobalLock(hDIB);
    GetDIBits(hdcWindow, hbmScreen, 0,
        (UINT)bmpScreen.bmHeight,
        lpbitmap,
        (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    HANDLE hFile = CreateFileA(filename.c_str(),
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, NULL);

    DWORD dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);
    bmfHeader.bfSize = dwSizeofDIB;
    bmfHeader.bfType = 0x4D42;

    DWORD dwBytesWritten = 0;
    WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
    WriteFile(hFile, (LPSTR)&bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
    WriteFile(hFile, (LPSTR)lpbitmap, dwBmpSize, &dwBytesWritten, NULL);

    GlobalUnlock(hDIB);
    GlobalFree(hDIB);
    CloseHandle(hFile);

    DeleteObject(hbmScreen);
    DeleteObject(hdcMemDC);
    ReleaseDC(NULL, hdcScreen);
    ReleaseDC(hwnd, hdcWindow);
}

// t800 Comments: Function to start recording a video of the specified window
void StartVideoRecording(const std::wstring& windowTitle, const std::string& outputFileName, int duration) {
    HWND hwnd = FindWindowW(NULL, windowTitle.c_str());
    if (!hwnd) {
        std::cerr << "Error: Could not find window with title \"" << wstringToString(windowTitle) << "\"" << std::endl;
        return;
    }

    RECT rect;
    GetClientRect(hwnd, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    std::string command = "ffmpeg.exe -y -f gdigrab -framerate 30 -t " + std::to_string(duration) +
        " -i title=\"" + wstringToString(windowTitle) + "\" -video_size " +
        std::to_string(width) + "x" + std::to_string(height) +
        " -codec:v libx264 -preset ultrafast -crf 18 " + outputFileName;

    std::cout << "Starting video recording: " << command << std::endl;
    system(command.c_str());
}

// t800 Comments: Function to find a window by its title (with retries)
HWND FindWindowWithRetry(const std::wstring& windowTitle, int retryCount = 10, int retryDelay = 1000) {
    HWND hwnd = NULL;
    for (int i = 0; i < retryCount; ++i) {
        hwnd = FindWindowW(NULL, windowTitle.c_str());
        if (hwnd) {
            break;
        }
        std::cerr << "Simple FindWindow: Window not found, retrying..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(retryDelay));
    }
    return hwnd;
}

// t800 Comments: Function to find a window that contains a specific substring in its title
HWND FindWindowContainingText(const std::wstring& windowText) {
    HWND hwnd = NULL;
    do {
        hwnd = FindWindowExW(NULL, hwnd, NULL, NULL);
        wchar_t title[256];
        GetWindowTextW(hwnd, title, sizeof(title) / sizeof(wchar_t));
        if (std::wstring(title).find(windowText) != std::wstring::npos) {
            return hwnd;
        }
    } while (hwnd != NULL);
    return NULL;
}

// t800 Comments: Function to capture full-screen screenshots every second
void CaptureFullScreenScreenshots(int duration) {
    HDC hdcScreen = GetDC(NULL);
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    for (int i = 0; i < duration; ++i) {
        std::string filename = "output/fullscreen_screenshot_" + std::to_string(i + 1) + ".png";
        CaptureScreenshot(NULL, filename); // t800 Comments: NULL HWND captures the full screen
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    ReleaseDC(NULL, hdcScreen);
}

// t800 Comments: Function to start full-screen video recording
void StartFullScreenRecording(const std::string& outputFileName, int duration) {
    std::string command = "ffmpeg.exe -y -f gdigrab -framerate 30 -t " + std::to_string(duration) +
        " -i desktop -video_size 1440x900 -codec:v libx264 -preset ultrafast -crf 18 " + outputFileName;

    std::cout << "Starting full-screen video recording: " << command << std::endl;
    system(command.c_str());
}

int main() {
    // t800 Comments: Step 1 - Capture screenshot and record video of the "Player" window
    std::wstring playerWindowTitle = L"Player";
    HWND playerHwnd = FindWindowWithRetry(playerWindowTitle);

    if (playerHwnd) {
        CaptureScreenshot(playerHwnd, "output/player_window_screenshot.png");
        StartVideoRecording(playerWindowTitle, "output/player_window_recording.mp4", 10);
    }
    else {
        std::cerr << "Error: Could not find 'Player' window." << std::endl;
    }

    // t800 Comments: Step 2 - Capture screenshot and record video of the window containing "Falco Engine" in its title
    std::wstring falcoEngineWindowText = L"Falco Engine";
    HWND falcoEngineHwnd = FindWindowContainingText(falcoEngineWindowText);

    if (falcoEngineHwnd) {
        wchar_t falcoEngineFullTitle[256];
        GetWindowTextW(falcoEngineHwnd, falcoEngineFullTitle, sizeof(falcoEngineFullTitle) / sizeof(wchar_t));
        std::wstring falcoEngineWindowTitle(falcoEngineFullTitle);

        CaptureScreenshot(falcoEngineHwnd, "output/falco_engine_window_screenshot.png");
        StartVideoRecording(falcoEngineWindowTitle, "output/falco_engine_window_recording.mp4", 10);
    }
    else {
        std::cerr << "Error: Could not find window containing text 'Falco Engine'." << std::endl;
    }

    // t800 Comments: Step 3 - Capture full-screen video and take screenshots every second
    int duration = 10;
    StartFullScreenRecording("output/fullscreen_capture.mp4", duration);
    CaptureFullScreenScreenshots(duration);

    return 0;
}
