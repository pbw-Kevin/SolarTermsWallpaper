/*
Solar Terms Wallpaper
(c) 2026 AIR-Kevin
AI declaration: using DeepSeek & GitHub Copilot in this project
*/

// #define SOLAR_TERMS_WALLPAPER_DEBUG 1

#include <string>
#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <thread>
#include <atomic>
#include <chrono>
#include <windows.h>
#include <shellapi.h>
#include <tlhelp32.h>
#include <direct.h>
#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")

const std::string version = "v0.1.0";

namespace Utils {
    std::string getCombinedPath(std::string a, std::string b);
    std::string getExePath(char* argv0);
    bool hasOtherProcessWithSameName(char* argv0);
    bool atLeastWindows10();
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
    int textPosX;
    int textPosY;
};

class Wallpaper{
    public:
        Wallpaper(std::string cwd, Logger* logger);
        void setWallpaper(int idx);
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
        std::string message = "二十四节气壁纸\n版本：" + version + "\n图片库版本：" + wallpaper->wallpaperVersion;
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
        // fscanf(confFile, "%259s %d %d\n", imageName, &conf[i].textPosX, &conf[i].textPosY);
        fscanf(confFile, "%259s\n", imageName);
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
    // int wideSize = MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, nullptr, 0);
    // std::wstring widePath(wideSize, L'\0');
    // MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, &widePath[0], wideSize);
    char pathBuf[MAX_PATH] = {};
    strcpy(pathBuf, path.c_str());
    // if (!SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, (PVOID)widePath.c_str(), SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE)) {
    if (!SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, pathBuf, SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE)) {
        DWORD errorCode = GetLastError();
        logger->log(Logger::Error, "Failed to set system wallpaper: %s", path.c_str());
        logger->log(Logger::Error, "Error code: %lu", errorCode);
        return true;
    }
    return false;
}

void Wallpaper::setWallpaper(int idx) {
    // If the same wallpaper is already active
    if (activeIdx == idx) {
        std::string currentWallpaper = getSystemWallpaper();
        if (currentWallpaper.empty() || currentWallpaper != Utils::getCombinedPath(cwd, conf[idx].imageName)) {
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
    if (setSystemWallpaper(imagePath)) return;

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
        if (solarTermIdx < 24) wallpaper->setWallpaper(solarTermIdx);
        else wallpaper->fallbackWallpaper();
        Sleep(10000);
    }

    TrayIcon::Stop();
    
    return 0;
}
