/*
Solar Terms Wallpaper
(c) 2026 AIR-Kevin
AI declaration: using DeepSeek & GitHub Copilot in this project
*/

// #define SOLAR_TERMS_WALLPAPER_DEBUG 1

#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <thread>
#include <atomic>
#include <chrono>
#include <algorithm>
#include <codecvt>
#include <locale>
#include <windows.h>
#include <shellapi.h>
#include <tlhelp32.h>
#include <direct.h>
#include <shlwapi.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#pragma comment(lib, "shlwapi.lib")

const std::string version = "v0.1.0";

namespace Utils {
    std::string getCombinedPath(std::string a, std::string b);
    std::string getExePath(char* argv0);
    bool hasOtherProcessWithSameName(char* argv0);
    bool atLeastWindows10();
    size_t deleteFilesStartingWithProcessed(const std::string& folderPath);
    void msgBoxShowMessage(std::string message);
    void Exit(int code);

    std::string getCombinedPath(std::string a, std::string b) {
        char combined[MAX_PATH];
        PathCombine(combined, a.c_str(), b.c_str());
        return combined;
    }

    std::string getExePath(char* argv0) {
        std::string exePath = argv0;
        return exePath.substr(0, exePath.find_last_of("\\"));
    }

    bool hasOtherProcessWithSameName(char* argv0) {
        std::string exeName = argv0;
        exeName = exeName.substr(exeName.find_last_of("\\") + 1);
        
        PROCESSENTRY32 entry;
        entry.dwSize = sizeof(PROCESSENTRY32);
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

        bool hasOne = false;

        if (Process32First(snapshot, &entry) == TRUE) {
            do {
                if (std::string(entry.szExeFile) == exeName) {
                    if (!hasOne) {
                        hasOne = true;
                        continue;
                    }
                    CloseHandle(snapshot);
                    return true;
                }
            } while (Process32Next(snapshot, &entry) == TRUE);
        }
        
        return false;
    }

    bool atLeastWindows10() {
        HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
        if (hNtdll) {
            using RtlGetVersionFunc = LONG(WINAPI*)(PRTL_OSVERSIONINFOEXW);
            auto RtlGetVersion = reinterpret_cast<RtlGetVersionFunc>(
                GetProcAddress(hNtdll, "RtlGetVersion"));
            if (RtlGetVersion) {
                OSVERSIONINFOEXW osvi = { sizeof(osvi) };
                if (RtlGetVersion(reinterpret_cast<PRTL_OSVERSIONINFOEXW>(&osvi)) == 0) {
                    return osvi.dwMajorVersion >= 10;
                }
            }
        }

        OSVERSIONINFOEXW osvi = { sizeof(osvi) };
        if (GetVersionExW(reinterpret_cast<LPOSVERSIONINFOW>(&osvi))) {
            return osvi.dwMajorVersion >= 10;
        }

        return false;
    }

    size_t deleteFilesStartingWithProcessed(const std::string& folderPath) {
        size_t deletedCount = 0;

        std::string searchPattern = folderPath + "\\processed*";

        WIN32_FIND_DATAA findData;
        HANDLE hFind = FindFirstFileA(searchPattern.c_str(), &findData);
        if (hFind == INVALID_HANDLE_VALUE) {
            DWORD err = GetLastError();
            if (err == ERROR_FILE_NOT_FOUND) {
                return 0;
            } else {
                // Error
            }
            return 0;
        }

        do {
            // 排除目录
            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                std::string fullPath = folderPath + "\\" + findData.cFileName;
                if (DeleteFileA(fullPath.c_str())) {
                    ++deletedCount;
                } else {
                    DWORD err = GetLastError();
                    // Error
                }
            }
        } while (FindNextFileA(hFind, &findData) != 0);

        FindClose(hFind);
        return deletedCount;
    }

    void msgBoxShowMessage(std::string message) {
        MessageBoxA(NULL, message.c_str(), "二十四节气壁纸", MB_OK | MB_ICONINFORMATION);
    }

    void Exit(int code) {
        exit(code);
    }
}

namespace SolarTerms {
    // Reference: https://zhuanlan.zhihu.com/p/8561213726
    const float c[24][2] = {
        {6.11, 5.4055},
        {20.84, 20.12},
        {4.6295, 3.87},
        {19.4599, 18.73},
        {6.3826, 5.63},
        {21.4155, 20.646},
        {5.59, 4.81},
        {20.888, 20.1},
        {6.318, 5.52},
        {21.86, 21.04},
        {6.5, 5.678},
        {22.2, 21.37},
        {7.928, 7.108},
        {23.65, 22.83},
        {28.35, 7.5},
        {23.95, 23.13},
        {8.44, 7.646},
        {23.822, 23.042},
        {9.098, 8.318},
        {24.218, 23.438},
        {8.218, 7.438},
        {23.08, 22.36},
        {7.9, 7.18},
        {22.6, 21.94}
    };

    const int ajust[22][3] = {
        {1982, 0, 1},
        {2019, 0, -1},
        {2000, 1, 1},
        {2082, 1, 1},
        {2026, 3, -1},
        {2084, 5, 1},
        {1911, 8, 1},
        {2008, 9, 1},
        {1902, 10, 1},
        {1928, 11, 1},
        {1925, 12, 1},
        {2016, 12, 1},
        {1922, 13, 1},
        {2002, 14, 1},
        {1927, 16, 1},
        {1942, 17, 1},
        {2089, 19, 1},
        {2089, 20, 1},
        {1978, 21, 1},
        {1954, 22, 1},
        {1918, 23, -1},
        {2021, 23, -1}
    };

    const int getSolarTerm(int yyyy, int mm, int dd) {
        int cidx;
        if(yyyy > 2000)
            cidx = 1;
        else if(yyyy > 1900)
            cidx = 0;
        else
            return 24;

        int jqidx1 = (mm - 1)*2;
        int jqidx2 = (mm - 1)*2 + 1;

        float c1 = c[jqidx1][cidx];
        float c2 = c[jqidx2][cidx];

        int d1 = 0;
        int d2 = 0;

        if(jqidx1 > 3)
            d1 = (yyyy%100) * 0.2422 + c1 - (yyyy%100)/4;
        else
            d1 = (yyyy%100) * 0.2422 + c1 - (yyyy%100 - 1)/4;

        if(jqidx2 > 3)
            d2 = (yyyy%100) * 0.2422 + c2 - (yyyy%100)/4;
        else
            d2 = (yyyy%100) * 0.2422 + c2 - (yyyy%100 - 1)/4;

        for(int i=0;i < sizeof(ajust)/(sizeof(int)*3);++i) {
            if(ajust[i][0] == yyyy && ajust[i][1] == jqidx1) {
                d1 += ajust[i][2];
            }
            if(ajust[i][0] == yyyy && ajust[i][1] == jqidx2) {
                d2 += ajust[i][2];
            }
        }

        if(dd == d1)
            return jqidx1;
        else if(dd == d2)
            return jqidx2;

        return 24;
    }

    const std::string SolarTermsNames[25] =
    {
        std::string("小寒"),
        std::string("大寒"),
        std::string("立春"),
        std::string("雨水"),
        std::string("惊蛰"),
        std::string("春分"),
        std::string("清明"),
        std::string("谷雨"),
        std::string("立夏"),
        std::string("小满"),
        std::string("芒种"),
        std::string("夏至"),
        std::string("小暑"),
        std::string("大暑"),
        std::string("立秋"),
        std::string("处暑"),
        std::string("白露"),
        std::string("秋分"),
        std::string("寒露"),
        std::string("霜降"),
        std::string("立冬"),
        std::string("小雪"),
        std::string("大雪"),
        std::string("冬至"),
        std::string("未知")
    };
}

class Logger {
    public:
        Logger(FILE* fp, int level = Warning);
        ~Logger();
        enum LogLevel {
            Debug = 1,
            Info,
            Warning,
            Error,
            Critical,
            None
        };
        static std::string getTime();
        void setLevel(int level);
        int getLevel();
        template<typename... Args>
        void log(int level, std::string content, Args... args);

    private:
        FILE* fp;
        int curLevel = Warning;
};

struct WallpaperConfig {
    std::string imageName;
    int fontSize;
    int left, top;
    int r, g, b;
    int alpha;
    bool isBold;
};

class Wallpaper{
    public:
        Wallpaper(std::string cwd, Logger* logger);
        void setWallpaper(int idx, std::string parentCwd, std::string dateString, std::string rawDateString);
        void fallbackWallpaper();
        std::string wallpaperVersion;
    private:
        Logger* logger;
        std::string cwd;
        WallpaperConfig conf[25];
        int activeIdx = -1;
        std::string getFallbackPath();
        bool setFallbackPath(std::string path);
        std::string getSystemWallpaper();
        bool setSystemWallpaper(std::string path);
};

Logger *logger;
Wallpaper *wallpaper;

namespace TrayIcon {
    constexpr UINT WM_TRAYICON = WM_USER + 1;
    constexpr int IDM_ABOUT = 1001;
    constexpr int IDM_EXIT = 1002;

    std::atomic<bool> g_bExitThread(false);
    HWND g_hTrayWnd = nullptr;
    HINSTANCE g_hInst = nullptr;
    HANDLE g_hThread = nullptr;

    std::string iconPath = "favicon.ico";

    LRESULT CALLBACK TrayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    DWORD WINAPI ThreadProc(LPVOID lpParam);
    bool CreateTrayIcon(HWND hwnd);
    void RemoveTrayIcon(HWND hwnd);
    void ShowContextMenu(HWND hwnd);
    void ShowAboutDialog(HWND hwnd);

    bool Start(HINSTANCE hInstance, std::string cwd = "")
    {
        if (g_hThread != nullptr)
            return true;

        iconPath = Utils::getCombinedPath(cwd, "assets/favicon.ico");

        g_hInst = hInstance;
        g_bExitThread = false;
        g_hThread = CreateThread(nullptr, 0, ThreadProc, nullptr, 0, nullptr);
        if (!g_hThread)
            return false;
        return true;
    }

    void Stop()
    {
        if (g_hThread == nullptr)
            return;

        g_bExitThread = true;
        
        if (g_hTrayWnd)
            PostMessage(g_hTrayWnd, WM_QUIT, 0, 0);

        WaitForSingleObject(g_hThread, INFINITE);
        CloseHandle(g_hThread);
        g_hThread = nullptr;
        g_hTrayWnd = nullptr;
    }

    bool IsRunning()
    {
        return g_hThread != nullptr;
    }

    DWORD WINAPI ThreadProc(LPVOID /*lpParam*/)
    {
        WNDCLASSEXA wc = { sizeof(WNDCLASSEXA) };
        wc.lpfnWndProc = TrayWndProc;
        wc.hInstance = g_hInst;
        wc.lpszClassName = "TrayAppClass";
        wc.hIcon = LoadIconA(nullptr, IDI_APPLICATION);
        wc.hCursor = LoadCursorA(nullptr, IDC_ARROW);

        if (!RegisterClassExA(&wc))
            return 1;

        HWND hwnd = CreateWindowExA(0, wc.lpszClassName, "TrayWindow", WS_OVERLAPPED,
                                    0, 0, 0, 0, nullptr, nullptr, g_hInst, nullptr);
        if (!hwnd)
        {
            UnregisterClassA("TrayAppClass", g_hInst);
            return 1;
        }

        g_hTrayWnd = hwnd;

        MSG msg;
        while (!g_bExitThread)
        {
            if (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessageA(&msg);
                if (msg.message == WM_QUIT)
                    break;
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }

        if (g_hTrayWnd)
        {
            DestroyWindow(g_hTrayWnd);
            g_hTrayWnd = nullptr;
        }
        UnregisterClassA("TrayAppClass", g_hInst);
        return 0;
    }

    bool CreateTrayIcon(HWND hwnd)
    {
        NOTIFYICONDATAA nid = {};
        nid.cbSize = sizeof(NOTIFYICONDATAA);
        nid.hWnd = hwnd;
        nid.uID = 1;
        nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        nid.uCallbackMessage = WM_TRAYICON;

        HICON hIcon = (HICON)LoadImageA(g_hInst, iconPath.c_str(), IMAGE_ICON, 128, 128, LR_LOADFROMFILE);
        if (!hIcon)
            hIcon = LoadIconA(nullptr, IDI_APPLICATION);
        nid.hIcon = hIcon;

        strcpy_s(nid.szTip, "二十四节气壁纸");
        return Shell_NotifyIconA(NIM_ADD, &nid) != FALSE;
    }

    void RemoveTrayIcon(HWND hwnd)
    {
        NOTIFYICONDATAA nid = {};
        nid.cbSize = sizeof(NOTIFYICONDATAA);
        nid.hWnd = hwnd;
        nid.uID = 1;
        Shell_NotifyIconA(NIM_DELETE, &nid);
    }

    void ShowContextMenu(HWND hwnd)
    {
        HMENU hMenu = CreatePopupMenu();
        AppendMenuA(hMenu, MF_STRING, IDM_ABOUT, "关于");
        AppendMenuA(hMenu, MF_STRING, IDM_EXIT, "退出应用");

        POINT pt;
        GetCursorPos(&pt);
        SetForegroundWindow(hwnd);
        TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, nullptr);
        PostMessage(hwnd, WM_NULL, 0, 0);
        DestroyMenu(hMenu);
    }

    void ShowAboutDialog(HWND hwnd)
    {
        std::string message = "二十四节气壁纸\n版本：" + version + "\n图片库版本：" + wallpaper->wallpaperVersion + "\n基于 MIT 协议在 GitHub 上开源：\nhttps://github.com/pbw-Kevin/SolarTermsWallpaper";
        MessageBoxA(hwnd, message.c_str(), "关于", MB_OK | MB_ICONINFORMATION);
    }

    LRESULT CALLBACK TrayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch (msg)
        {
        case WM_CREATE:
            if (!CreateTrayIcon(hwnd))
                MessageBoxA(hwnd, "无法创建托盘图标", "错误", MB_OK | MB_ICONERROR);
            break;
        case WM_TRAYICON:
            if (lParam == WM_RBUTTONUP)
                ShowContextMenu(hwnd);
            break;
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
            case IDM_ABOUT:
                ShowAboutDialog(hwnd);
                break;
            case IDM_EXIT:
                DestroyWindow(hwnd);
                Utils::Exit(0);
                break;
            }
            break;
        case WM_DESTROY:
            RemoveTrayIcon(hwnd);
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProcA(hwnd, msg, wParam, lParam);
        }
        return 0;
    }
} // namespace TrayIcon

namespace ImageProcess {

    struct Image {
        std::vector<unsigned char> data;   // RGBA
        int width;
        int height;
    };

    static std::wstring GB2312ToWide(const std::string& str) {
        if (str.empty()) return L"";
        int len = MultiByteToWideChar(936, 0, str.c_str(), (int)str.size(), nullptr, 0);
        if (len == 0) return L"";
        std::wstring result(len, L'\0');
        MultiByteToWideChar(936, 0, str.c_str(), (int)str.size(), &result[0], len);
        return result;
    }

    inline Image loadImage(const std::string& filename) {
        Image img;
        int channels;
        unsigned char* raw = stbi_load(filename.c_str(), &img.width, &img.height, &channels, 4);
        if (!raw) {
            if (logger) logger->log(Logger::Error, "Failed to load image: %s", filename.c_str());
            img.width = img.height = 0;
        } else {
            img.data.assign(raw, raw + img.width * img.height * 4);
            stbi_image_free(raw);
        }
        return img;
    }

    inline bool saveImage(const Image& img, const std::string& filename) {
        if (img.data.empty() || img.width <= 0 || img.height <= 0) {
            if (logger) logger->log(Logger::Error, "Invalid image data");
            return false;
        }
        std::string ext = filename.substr(filename.find_last_of('.') + 1);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        int ret = 0;
        if (ext == "png") {
            ret = stbi_write_png(filename.c_str(), img.width, img.height, 4, img.data.data(), img.width * 4);
        } else if (ext == "jpg" || ext == "jpeg") {
            ret = stbi_write_jpg(filename.c_str(), img.width, img.height, 4, img.data.data(), 90);
        } else if (ext == "bmp") {
            ret = stbi_write_bmp(filename.c_str(), img.width, img.height, 4, img.data.data());
        } else if (ext == "tga") {
            ret = stbi_write_tga(filename.c_str(), img.width, img.height, 4, img.data.data());
        } else {
            if (logger) logger->log(Logger::Error, "Unsupported format: %s", ext.c_str());
            return false;
        }
        if (ret == 0) {
            if (logger) logger->log(Logger::Error, "Failed to save image: %s", filename.c_str());
            return false;
        }
        return true;
    }

    inline bool drawTextOnImage(Image& img,
                                const std::string& text,
                                const std::string& fontPath,
                                int fontSize,
                                int left, int top,
                                int r, int g, int b,
                                int alpha = 255) {
        if (img.data.empty() || text.empty() || fontPath.empty() || fontSize <= 0) {
            if (logger) logger->log(Logger::Error, "drawTextOnImage: invalid parameters");
            return false;
        }

        std::wstring wtext = GB2312ToWide(text);
        if (wtext.empty() && !text.empty()) {
            if (logger) logger->log(Logger::Error, "drawTextOnImage: text conversion failed");
            return false;
        }

        FILE* fontFile = fopen(fontPath.c_str(), "rb");
        if (!fontFile) {
            if (logger) logger->log(Logger::Error, "drawTextOnImage: cannot open font %s", fontPath.c_str());
            return false;
        }
        fseek(fontFile, 0, SEEK_END);
        long fsize = ftell(fontFile);
        fseek(fontFile, 0, SEEK_SET);
        std::vector<unsigned char> fontBuffer(fsize);
        if (fread(fontBuffer.data(), 1, fsize, fontFile) != (size_t)fsize) {
            if (logger) logger->log(Logger::Error, "drawTextOnImage: read font error");
            fclose(fontFile);
            return false;
        }
        fclose(fontFile);

        stbtt_fontinfo fontInfo;
        if (!stbtt_InitFont(&fontInfo, fontBuffer.data(), 0)) {
            if (logger) logger->log(Logger::Error, "drawTextOnImage: font init failed");
            return false;
        }

        float scale = stbtt_ScaleForPixelHeight(&fontInfo, (float)fontSize);

        int bbox_xmin = 0, bbox_ymin = 0, bbox_xmax = 0, bbox_ymax = 0;
        int cursorX = 0;
        for (size_t i = 0; i < wtext.size(); ++i) {
            int advance, lsb;
            stbtt_GetCodepointHMetrics(&fontInfo, wtext[i], &advance, &lsb);
            int x0, y0, x1, y1;
            stbtt_GetCodepointBitmapBox(&fontInfo, wtext[i], scale, scale, &x0, &y0, &x1, &y1);
            int char_left   = cursorX + x0;
            int char_right  = cursorX + x1;
            int char_top    = y0;
            int char_bottom = y1;
            if (i == 0) {
                bbox_xmin = char_left;
                bbox_xmax = char_right;
                bbox_ymin = char_top;
                bbox_ymax = char_bottom;
            } else {
                bbox_xmin = (std::min)(bbox_xmin, char_left);
                bbox_xmax = (std::max)(bbox_xmax, char_right);
                bbox_ymin = (std::min)(bbox_ymin, char_top);
                bbox_ymax = (std::max)(bbox_ymax, char_bottom);
            }
            cursorX += (int)(advance * scale);
        }

        int offsetX = left - bbox_xmin;
        int offsetY = top - bbox_ymin;

        cursorX = 0;
        for (size_t i = 0; i < wtext.size(); ++i) {
            int advance, lsb;
            stbtt_GetCodepointHMetrics(&fontInfo, wtext[i], &advance, &lsb);
            int x0, y0, x1, y1;
            stbtt_GetCodepointBitmapBox(&fontInfo, wtext[i], scale, scale, &x0, &y0, &x1, &y1);
            int glyph_w = x1 - x0;
            int glyph_h = y1 - y0;
            if (glyph_w <= 0 || glyph_h <= 0) {
                cursorX += (int)(advance * scale);
                continue;
            }

            std::vector<unsigned char> bitmap(glyph_w * glyph_h, 0);
            stbtt_MakeCodepointBitmap(&fontInfo, bitmap.data(),
                                      glyph_w, glyph_h, glyph_w,
                                      scale, scale, wtext[i]);

            int destX = cursorX + x0 + offsetX;
            int destY = y0 + offsetY;

            for (int py = 0; py < glyph_h; ++py) {
                for (int px = 0; px < glyph_w; ++px) {
                    int imgX = destX + px;
                    int imgY = destY + py;
                    if (imgX >= 0 && imgX < img.width && imgY >= 0 && imgY < img.height) {
                        unsigned char mask = bitmap[py * glyph_w + px];
                        if (mask == 0) continue;
                        float srcAlpha = (mask / 255.0f) * (alpha / 255.0f);
                        float dstAlpha = 1.0f - srcAlpha;
                        int idx = (imgY * img.width + imgX) * 4;
                        img.data[idx + 0] = (unsigned char)(img.data[idx + 0] * dstAlpha + r * srcAlpha);
                        img.data[idx + 1] = (unsigned char)(img.data[idx + 1] * dstAlpha + g * srcAlpha);
                        img.data[idx + 2] = (unsigned char)(img.data[idx + 2] * dstAlpha + b * srcAlpha);
                        img.data[idx + 3] = (unsigned char)(img.data[idx + 3] * dstAlpha + alpha * srcAlpha);
                    }
                }
            }
            cursorX += (int)(advance * scale);
        }
        return true;
    }

    inline Image createTextImage(const std::string& text,
                                 const std::string& fontPath,
                                 int fontSize,
                                 int r, int g, int b,
                                 int alpha = 255,
                                 int extraLeft = 0, int extraTop = 0,
                                 int extraRight = 0, int extraBottom = 0) {
        Image result;
        if (text.empty() || fontPath.empty()) {
            if (logger) logger->log(Logger::Error, "createTextImage: empty text or font");
            return result;
        }

        std::wstring wtext = GB2312ToWide(text);
        FILE* fontFile = fopen(fontPath.c_str(), "rb");
        if (!fontFile) {
            if (logger) logger->log(Logger::Error, "createTextImage: cannot open font %s", fontPath.c_str());
            return result;
        }
        fseek(fontFile, 0, SEEK_END);
        long fsize = ftell(fontFile);
        fseek(fontFile, 0, SEEK_SET);
        std::vector<unsigned char> fontBuffer(fsize);
        fread(fontBuffer.data(), 1, fsize, fontFile);
        fclose(fontFile);

        stbtt_fontinfo fontInfo;
        if (!stbtt_InitFont(&fontInfo, fontBuffer.data(), 0)) {
            if (logger) logger->log(Logger::Error, "createTextImage: font init failed");
            return result;
        }

        float scale = stbtt_ScaleForPixelHeight(&fontInfo, (float)fontSize);
        int bbox_xmin = 0, bbox_ymin = 0, bbox_xmax = 0, bbox_ymax = 0;
        int cursorX = 0;
        for (size_t i = 0; i < wtext.size(); ++i) {
            int advance, lsb;
            stbtt_GetCodepointHMetrics(&fontInfo, wtext[i], &advance, &lsb);
            int x0, y0, x1, y1;
            stbtt_GetCodepointBitmapBox(&fontInfo, wtext[i], scale, scale, &x0, &y0, &x1, &y1);
            int char_left   = cursorX + x0;
            int char_right  = cursorX + x1;
            int char_top    = y0;
            int char_bottom = y1;
            if (i == 0) {
                bbox_xmin = char_left;
                bbox_xmax = char_right;
                bbox_ymin = char_top;
                bbox_ymax = char_bottom;
            } else {
                bbox_xmin = (std::min)(bbox_xmin, char_left);
                bbox_xmax = (std::max)(bbox_xmax, char_right);
                bbox_ymin = (std::min)(bbox_ymin, char_top);
                bbox_ymax = (std::max)(bbox_ymax, char_bottom);
            }
            cursorX += (int)(advance * scale);
        }

        int width  = (bbox_xmax - bbox_xmin) + extraLeft + extraRight;
        int height = (bbox_ymax - bbox_ymin) + extraTop + extraBottom;
        if (width <= 0 || height <= 0) {
            if (logger) logger->log(Logger::Error, "createTextImage: invalid dimensions");
            return result;
        }

        result.width = width;
        result.height = height;
        result.data.assign(width * height * 4, 0);

        int textLeft = extraLeft - bbox_xmin;
        int textTop  = extraTop - bbox_ymin;
        if (!drawTextOnImage(result, text, fontPath, fontSize, textLeft, textTop, r, g, b, alpha)) {
            if (logger) logger->log(Logger::Error, "createTextImage: drawing failed");
            return Image{};
        }
        return result;
    }

} // namespace ImageProcess

Logger::Logger(FILE* fp, int level) : fp(fp) {
    if(level >= Debug && level <= None) this->curLevel = level;
    this->log(Info, "Logger initialized.");
};

Logger::~Logger() {
    fclose(fp);
}

std::string Logger::getTime() {
    time_t rawtime;
    time(&rawtime);
    char tmstr[9];
    strftime(tmstr, 9, "%H:%M:%S", localtime(&rawtime));
    return tmstr;
}

int Logger::getLevel() {
    return curLevel;
}

void Logger::setLevel(int level) {
    if(level >= Debug && level <= None) curLevel = level;
}

template<typename... Args>
void Logger::log(int level, std::string content, Args... args) {
    if(level < curLevel || level >= None) return;
    std::string levelStr;
    if(level == Debug) levelStr = "DEBUG";
    else if(level == Info) levelStr = "INFO";
    else if(level == Warning) levelStr = "WARNING";
    else if(level == Error) levelStr = "ERROR";
    else if(level == Critical) levelStr = "CRITICAL";
    fprintf(fp, ("[" + getTime() + "|" + levelStr + "] " + content + "\n").c_str(), args...);
    fflush(fp);
}

Wallpaper::Wallpaper(std::string cwd, Logger* logger) : cwd(cwd), logger(logger) {
    logger->log(Logger::Info, "Initializing Wallpaper...");
    std::string confPath = Utils::getCombinedPath(cwd, "CONFIG");
    if (!PathFileExists(confPath.c_str())) {
        logger->log(Logger::Error, "CONFIG file not found: %s", confPath);
        logger->log(Logger::Info, "Wallpaper initialized.");
        return;
    }
    FILE* confFile = fopen(confPath.c_str(), "r");
    char* wallpaperVersionBuf = new char[256];
    fscanf(confFile, "%s\n", wallpaperVersionBuf);
    wallpaperVersion = wallpaperVersionBuf;
    for (int i = 0; i < 24; i++) {
        char imageName[MAX_PATH];
        int isBold;
        fscanf(confFile, "%259s %d %d %d %d %d %d %d %d\n", imageName, &conf[i].fontSize, &conf[i].left, &conf[i].top, &conf[i].r, &conf[i].g, &conf[i].b, &conf[i].alpha, &isBold);
        conf[i].isBold = isBold;
        conf[i].imageName = imageName;
    }
    logger->log(Logger::Info, "Wallpaper initialized.");
}

std::string Wallpaper::getFallbackPath() {
    char fallbackPath[MAX_PATH] = {};
    std::string fallbackPathFilePath = Utils::getCombinedPath(cwd, "fallback_path.txt");
    FILE* fallbackPathFile = fopen(fallbackPathFilePath.c_str(), "r");
    if (fallbackPathFile) {
        fscanf(fallbackPathFile, "%259s", fallbackPath);
        fclose(fallbackPathFile);
    }
    return fallbackPath;
}

bool Wallpaper::setFallbackPath(std::string path) {
    std::string fallbackPathFilePath = Utils::getCombinedPath(cwd, "fallback_path.txt");
    FILE* fallbackPathFile = fopen(fallbackPathFilePath.c_str(), "w");
    if (fallbackPathFile) {
        fprintf(fallbackPathFile, "%s", path.c_str());
        fclose(fallbackPathFile);
        return false;
    } else {
        logger->log(Logger::Error, "Failed to write fallback path file: %s", fallbackPathFilePath);
        return true;
    }
}

std::string Wallpaper::getSystemWallpaper() {
    char systemWallpaper[MAX_PATH] = {};
    SystemParametersInfo(SPI_GETDESKWALLPAPER, MAX_PATH, systemWallpaper, 0);
    return systemWallpaper;
}

bool Wallpaper::setSystemWallpaper(std::string path) {
    char pathBuf[MAX_PATH] = {};
    strcpy(pathBuf, path.c_str());
    if (!SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, pathBuf, SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE)) {
        DWORD errorCode = GetLastError();
        logger->log(Logger::Error, "Failed to set system wallpaper: %s", path.c_str());
        logger->log(Logger::Error, "Error code: %lu", errorCode);
        return true;
    }
    return false;
}

void Wallpaper::setWallpaper(int idx, std::string parentCwd, std::string dateString, std::string rawDateString) {
    std::string processedImagePath = Utils::getCombinedPath(cwd, "processed_" + rawDateString + ".png");

    // If the same wallpaper is already active
    if (activeIdx == idx) {
        std::string currentWallpaper = getSystemWallpaper();
        if (currentWallpaper.empty() || currentWallpaper != processedImagePath) {
            logger->log(Logger::Warning, "Active wallpaper is missing or changed, resetting wallpaper.");
            setFallbackPath(currentWallpaper);
        } else {
            logger->log(Logger::Info, "Active wallpaper is already set, no action needed.");
        }
        return;
    }

    // Load exist wallpaper path for fallback
    std::string fallbackPath = getFallbackPath();

    // If no fallback path, get current wallpaper as fallback
    if (fallbackPath.empty()) fallbackPath = getSystemWallpaper();

    // Set new wallpaper
    std::string imagePath = Utils::getCombinedPath(cwd, conf[idx].imageName);
    if (!PathFileExists(imagePath.c_str())) {
        logger->log(Logger::Error, "Wallpaper image not found: %s", imagePath);
        return;
    }

    ImageProcess::Image img = ImageProcess::loadImage(imagePath);

    if (img.data.empty()) {
        logger->log(Logger::Error, "Failed to load wallpaper image: %s", imagePath);
        return;
    }

    if (!conf[idx].imageName.empty()) {
        if (!ImageProcess::drawTextOnImage(img, dateString, Utils::getCombinedPath(parentCwd, conf[idx].isBold ? "assets\\bold.otf" : "assets\\regular.otf"), conf[idx].fontSize, conf[idx].left, conf[idx].top, conf[idx].r, conf[idx].g, conf[idx].b, conf[idx].alpha)) {
            logger->log(Logger::Error, "Failed to draw text on wallpaper image: %s", imagePath);
            return;
        }
    }

    Utils::deleteFilesStartingWithProcessed(cwd);

    if (!ImageProcess::saveImage(img, processedImagePath)) {
        logger->log(Logger::Error, "Failed to save processed wallpaper image: %s", processedImagePath);
        return;
    }

    if (setSystemWallpaper(processedImagePath)) return;

    // Update idx
    activeIdx = idx;

    // Save current wallpaper path for fallback
    setFallbackPath(fallbackPath.empty() ? "EMPTY_PATH" : fallbackPath);
}

void Wallpaper::fallbackWallpaper() {
    // Get fallback path
    std::string fallbackPath = getFallbackPath();
    
    if (fallbackPath.empty()) return;

    // Set fallback wallpaper
    if (setSystemWallpaper(fallbackPath == "EMPTY_PATH" ? "" : fallbackPath)) return;

    activeIdx = -1;
    setFallbackPath("");
}

int main(int argc, char* argv[]) {
    HWND hWnd = GetConsoleWindow();
    ShowWindow(hWnd, SW_HIDE);

    if (Utils::hasOtherProcessWithSameName(argv[0])) {
        Utils::msgBoxShowMessage("二十四节气壁纸已经在运行了，无需重复启动。");
        Utils::Exit(0);
    }
    if (!Utils::atLeastWindows10()) {
        Utils::msgBoxShowMessage("二十四节气壁纸需要在 Windows 10 或更高版本上运行。\n即将退出程序。");
        Utils::Exit(1);
    }

    std::string cwd = Utils::getExePath(argv[0]);
    logger = new Logger(fopen(Utils::getCombinedPath(cwd, "log.txt").c_str(), "w"),
#ifdef SOLAR_TERMS_WALLPAPER_DEBUG
        Logger::Debug
#else
        Logger::Error
#endif
    );
    wallpaper = new Wallpaper(Utils::getCombinedPath(cwd, "wallpapers"), logger);

    HINSTANCE hInst = GetModuleHandleA(NULL);

    if (!TrayIcon::Start(hInst, cwd))
    {
        MessageBoxA(NULL, "无法启动托盘功能", "错误", MB_OK | MB_ICONERROR);
        Utils::Exit(1);
    }

    while(TrayIcon::IsRunning()){
        SYSTEMTIME st;
        GetLocalTime(&st);
        int solarTermIdx = SolarTerms::getSolarTerm(st.wYear, st.wMonth, st.wDay);
        logger->log(Logger::Info, "Current date: %04d-%02d-%02d, Solar Term: %s", st.wYear, st.wMonth, st.wDay, SolarTerms::SolarTermsNames[solarTermIdx].c_str());
        if (solarTermIdx < 24) wallpaper->setWallpaper(solarTermIdx, cwd, std::to_string(st.wYear) + " 年 " + std::to_string(st.wMonth) + " 月 " + std::to_string(st.wDay) + " 日", std::to_string(st.wYear) + "_" + std::to_string(st.wMonth) + "_" + std::to_string(st.wDay));
        else wallpaper->fallbackWallpaper();
        Sleep(10000);
    }

    TrayIcon::Stop();
    
    return 0;
}
