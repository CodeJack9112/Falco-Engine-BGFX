#include "DialogProgress.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <dear-imgui/misc/cpp/imgui_stdlib.h>

#include "../Windows/MainWindow.h"
#include "../Engine/Renderer/Renderer.h"
#include "../Engine/Classes/StringConverter.h"

#ifdef _WIN32
#include <CommCtrl.h>

namespace GX
{
    LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch (msg)
        {
        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_CTLCOLORSTATIC:
        {
            HDC hdcStatic = (HDC)wParam;
            //if (lParam == (LPARAM)staticTextFieldTwo)
            //{
            SetTextColor(hdcStatic, RGB(0, 0, 0));
            SetBkColor(hdcStatic, RGB(255, 255, 255));

            return (LRESULT)WHITE_BRUSH;
            //}
            break;
        }
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }
        return 0;
    }
}
#else
#include <iostream>
#include "../Engine/Assets/Font.h"
#include "../Engine/Assets/Texture.h"

namespace GX
{
    void* DialogProgress::fontSurf = nullptr;
    void* DialogProgress::fontData = nullptr;
    std::map<int, GlyphInfo> DialogProgress::glyphs;
}
#endif

#include <SDL2/SDL_video.h>

namespace GX
{
    void DialogProgress::show()
    {
        if (hwnd != nullptr)
            return;

    #ifdef _WIN32
        //Create window
        const char g_szClassName[] = "Progress Dialog Class";

        HINSTANCE hInstance = GetModuleHandle(NULL);

        WNDCLASSEX wc;
        MSG Msg;

        //Step 1: Registering the Window Class
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = 0;
        wc.lpfnWndProc = WndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = hInstance;
        wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.lpszMenuName = NULL;
        wc.lpszClassName = g_szClassName;
        wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

        RegisterClassEx(&wc);

        //if (!RegisterClassEx(&wc))
        //    return;

        int ScreenX = GetSystemMetrics(SM_CXSCREEN);
        int ScreenY = GetSystemMetrics(SM_CYSCREEN);

        // Step 2: Creating the Window
        hwnd = CreateWindowEx(
            WS_EX_CLIENTEDGE,
            g_szClassName,
            title.c_str(),
            WS_OVERLAPPED | WS_CAPTION | WS_DLGFRAME,
            ScreenX / 2 - 200, ScreenY / 2 - 100, 400, 120,
            NULL, NULL, hInstance, NULL);

        if (hwnd != NULL)
        {
            ShowWindow((HWND)hwnd, SW_SHOW);

            //Text 1
            text1Hwnd = CreateWindow("static", "Text 1",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                10, 10, 400 - 40, 20,
                (HWND)hwnd, (HMENU)(501),
                hInstance, NULL);

            HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
            SendMessage((HWND)text1Hwnd, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

            SetWindowText((HWND)text1Hwnd, statusText1.c_str());

            //Progress 1
            progress1Hwnd = CreateWindowEx(0, PROGRESS_CLASS, (LPTSTR)NULL,
                WS_CHILD | WS_VISIBLE, 10,
                40,
                400 - 40,
                24,
                (HWND)hwnd, (HMENU)0, hInstance, NULL);

            SendMessage((HWND)progress1Hwnd, PBM_SETRANGE, 0, (LPARAM)MAKELONG(0, 100));
            SendMessage((HWND)progress1Hwnd, PBM_SETSTEP, (WPARAM)1, 0);

            setProgress(progress1, 0);

            UpdateWindow((HWND)hwnd);

            if (twoProgressBars)
                createSecondProgressBar();
        }
    #else
        hwnd = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 400, 120, 0);

        Font* fnt = Font::getDefaultFont();

        if (fontSurf == nullptr)
        {
            FontAtlas* atlas = fnt->getFontAtlas(11.0f, [=](void* data, int size, Texture* atlasTex)
            {
                fontData = new unsigned char[size];
                memcpy(fontData, data, size);
                fontSurf = SDL_CreateRGBSurfaceFrom(fontData, atlasTex->getWidth(), atlasTex->getHeight(), 32, atlasTex->getWidth() * 4, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
            });

            glyphs = atlas->glyphs;
        }

        setProgress(progress1, 0);

        if (twoProgressBars)
            createSecondProgressBar();
    #endif

        visible = true;
        progress1 = 0;
        progress2 = 0;
    }

    #ifndef _WIN32
        void DialogProgress::drawText(std::string text, int x, int y)
        {
            SDL_Surface* surf = SDL_GetWindowSurface((SDL_Window*)hwnd);

            int cur = 0;
            for (int i = 0; i < text.size(); ++i)
            {
                GlyphInfo g = glyphs[(int)text[i]];

                if (text[i] == ' ')
                {
                    g = glyphs[(int)'i'];
                }
                else
                {
                    SDL_Rect fRect = { (int)g.rect.x, (int)g.rect.y, (int)g.size.x, (int)g.size.y };
                    SDL_Rect dRect = { (int)(x - g.xOffset + cur), (int)(y - g.yOffset), (int)(x - g.xOffset + cur + g.size.x), (int)(y - g.yOffset + g.size.y) };

                    SDL_BlitSurface((SDL_Surface*)fontSurf, &fRect, surf, &dRect);
                }

                cur += g.size.x;
            }
        }

        void DialogProgress::updateWindow()
        {
            if (hwnd == nullptr)
                return;

            SDL_Surface* fnt = (SDL_Surface*)fontSurf;

            SDL_Surface* surf = SDL_GetWindowSurface((SDL_Window*)hwnd);
            uint32_t color = SDL_MapRGB(surf->format, 50, 50, 50);
            SDL_FillRect(surf, NULL, color);

            //Progress 1
            drawText(statusText1, 10, 30);

            SDL_Rect pb1r1 = { 10, 40, 400 - 20, 20 };
            SDL_Rect pb1r2 = { 10, 40, (int)(380.0f * progress1), 20 };
            uint32_t pb1c1 = SDL_MapRGB(surf->format, 70, 70, 70);
            uint32_t pb1c2 = SDL_MapRGB(surf->format, 20, 200, 40);
            SDL_FillRect(surf, &pb1r1, pb1c1);
            SDL_FillRect(surf, &pb1r2, pb1c2);

            if (twoProgressBars)
            {
                //Progress 2
                drawText(statusText2, 10, 94);

                SDL_Rect pb2r1 = { 10, 104, 400 - 20, 20 };
                SDL_Rect pb2r2 = { 10, 104, (int)(380.0f * progress2), 20 };
                uint32_t pb2c1 = SDL_MapRGB(surf->format, 70, 70, 70);
                uint32_t pb2c2 = SDL_MapRGB(surf->format, 20, 200, 40);
                SDL_FillRect(surf, &pb2r1, pb2c1);
                SDL_FillRect(surf, &pb2r2, pb2c2);
            }

            SDL_UpdateWindowSurface((SDL_Window*)hwnd);
        }
    #endif

    void DialogProgress::hide()
    {
        if (hwnd == nullptr)
            return;

        visible = false;

    #ifdef _WIN32
        DestroyWindow((HWND)hwnd);

        HINSTANCE hInst = GetModuleHandle(NULL);
        UnregisterClassW(L"Progress Dialog Class", hInst);
    #else
        SDL_DestroyWindow((SDL_Window*)hwnd);
    #endif

        hwnd = nullptr;
    }

    void DialogProgress::setStatusText(std::string text, int barIdx)
    {
        if (barIdx == 0)
        {
            statusText1 = text;
    #ifdef _WIN32
            SetWindowText((HWND)text1Hwnd, statusText1.c_str());
    #endif
        }
        else
        {
            if (twoProgressBars)
            {
                statusText2 = text;
    #ifdef _WIN32
                SetWindowText((HWND)text2Hwnd, statusText2.c_str());
    #endif
            }
        }

    #ifndef _WIN32
        updateWindow();
    #endif
    }

    void DialogProgress::setProgress(float p, int barIdx)
    {
        if (barIdx == 0)
        {
            progress1 = p;
    #ifdef _WIN32
            SendMessage((HWND)progress1Hwnd, PBM_SETPOS, (WPARAM)(p * 100.0f), 0);
    #endif
        }
        else
        {
            if (twoProgressBars)
            {
                progress2 = p;
    #ifdef _WIN32
                SendMessage((HWND)progress2Hwnd, PBM_SETPOS, (WPARAM)(p * 100.0f), 0);
    #endif
            }
        }

    #ifndef _WIN32
        updateWindow();
    #endif
    }

    void DialogProgress::setTitle(std::string text)
    {
        title = text;

        if (hwnd != NULL)
        {
    #ifdef _WIN32
            SetWindowText((HWND)hwnd, text.c_str());
    #else
            SDL_SetWindowTitle((SDL_Window*)hwnd, text.c_str());
    #endif
        }
    }

    void DialogProgress::setTwoProgressBars(bool value)
    {
        twoProgressBars = value;

        if (value)
        {
            if (text2Hwnd == NULL && progress2Hwnd == NULL)
                createSecondProgressBar();
        }
        else
        {
            destroySecondProgressBar();
        }
    }

    void DialogProgress::createSecondProgressBar()
    {
    #ifdef _WIN32
        RECT rect;
        GetWindowRect((HWND)hwnd, &rect);
        SetWindowPos((HWND)hwnd, NULL, 0, 0, rect.right - rect.left, rect.bottom - rect.top + 65, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

        HINSTANCE hInstance = GetModuleHandle(NULL);

        //Text 2
        text2Hwnd = CreateWindow("static", "Text 2",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP,
            10, 74, 400 - 40, 20,
            (HWND)hwnd, (HMENU)(502),
            hInstance, NULL);

        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        SendMessage((HWND)text2Hwnd, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

        SetWindowText((HWND)text2Hwnd, statusText2.c_str());

        //Progress 2
        progress2Hwnd = CreateWindowEx(0, PROGRESS_CLASS, (LPTSTR)NULL,
            WS_CHILD | WS_VISIBLE, 10,
            104,
            400 - 40,
            24,
            (HWND)hwnd, (HMENU)0, hInstance, NULL);

        SendMessage((HWND)progress2Hwnd, PBM_SETRANGE, 0, (LPARAM)MAKELONG(0, 100));
        SendMessage((HWND)progress2Hwnd, PBM_SETSTEP, (WPARAM)1, 0);

        setProgress(progress2, 1);

        UpdateWindow((HWND)hwnd);
    #else
        SDL_SetWindowSize((SDL_Window*)hwnd, 400, 185);
        setProgress(progress2, 1);
        updateWindow();
    #endif
    }

    void DialogProgress::destroySecondProgressBar()
    {
    #ifdef _WIN32
        if (text2Hwnd != NULL)
            DestroyWindow((HWND)text2Hwnd);

        if (progress2Hwnd != NULL)
            DestroyWindow((HWND)progress2Hwnd);

        RECT rect;
        GetWindowRect((HWND)hwnd, &rect);
        SetWindowPos((HWND)hwnd, NULL, 0, 0, rect.right - rect.left, rect.bottom - rect.top - 65, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

        UpdateWindow((HWND)hwnd);
    #else
        SDL_SetWindowSize((SDL_Window*)hwnd, 400, 125);
        updateWindow();
    #endif

        text2Hwnd = nullptr;
        progress2Hwnd = nullptr;
    }
}