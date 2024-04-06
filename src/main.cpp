#include <Windows.h>
#include <WinUser.h>
#include <ShlObj_core.h>
#include <cstdio>
#include <tchar.h>
#include <filesystem>
#include <iostream>


bool showFailMSGBox() {
    int msgboxID = MessageBoxW(
            NULL,
            (LPCWSTR)L"Failed to locate log folder.\nDo you want to continue?",
            (LPCWSTR)L"Error",
            MB_YESNO | MB_DEFBUTTON2
    );
    return msgboxID == IDYES;
}

std::string getLogFolderPath() {
    PWSTR pPath = NULL;
    std::string path;
    if (SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_DEFAULT, NULL, &pPath) == S_OK) {
        int wlen = lstrlenW(pPath);
        int len = WideCharToMultiByte(CP_ACP, 0, pPath, wlen, NULL, 0, NULL, NULL);
        if (len > 0){
            path.resize(len+1);
            WideCharToMultiByte(CP_ACP, 0, pPath, wlen, &path[0], len, NULL, NULL);
            path[len] = '\\';
        }
        CoTaskMemFree(pPath);
    }
    return path + R"(\Neverwinter Nights\logs\)";
}

bool rotateLog(bool safe = true) {
    std::string logFolder = getLogFolderPath();
    if (safe && (logFolder.empty() || !std::filesystem::exists(logFolder))) {
        std::cout << "Log folder " << std::endl;
        return showFailMSGBox();
    }
    std::filesystem::path oldLogPath = logFolder + "nwclientLog1.txt";
    if (!std::filesystem::exists(oldLogPath)) {
        std::cout << "Log file does not exist" << std::endl;
        return true;
    }
    time_t logTime = std::chrono::system_clock::to_time_t(std::chrono::clock_cast<std::chrono::system_clock>(std::filesystem::last_write_time(oldLogPath)));
    std::stringstream ss;
    ss << std::put_time(std::localtime(&logTime), "log_%Y-%m-%d_%H-%M-%S.txt");
    std::filesystem::path newLogPath = logFolder + ss.str();
    std::filesystem::rename(oldLogPath, newLogPath);
    std::cout << "Log file rotated" << std::endl;
    return true;
}

BOOL CALLBACK EnumThreadWndProc(HWND hWnd, LPARAM lParam) {
    DWORD dwStyle = GetWindowLong(hWnd, GWL_STYLE);
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    SetWindowPos(hWnd, HWND_TOP, 0, 0, w, h, SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    SetWindowLongPtr(hWnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
    return FALSE;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    if (!rotateLog()) {
        return 1;
    }

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.dwFlags = STARTF_USEPOSITION | STARTF_USESIZE;
    si.dwX = 0;
    si.dwY = 0;
    si.dwXSize = GetSystemMetrics(SM_CXSCREEN);
    si.dwYSize = GetSystemMetrics(SM_CYSCREEN);
    si.cb = sizeof(si);

    int size = strlen(lpCmdLine) + strlen("nwmain.exe") + 2;
    char *cmdline = new char[size];
    sprintf_s(cmdline, size, "%s %s", "nwmain.exe", lpCmdLine);


    if (!CreateProcess(NULL, cmdline, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        std::cout << "CreateProcess failed (" <<  GetLastError() << ")" << std::endl;
        return 1;
    }

    WaitForInputIdle(pi.hProcess, 3000);
    Sleep(2000);
    EnumThreadWindows(pi.dwThreadId, EnumThreadWndProc, NULL);
    WaitForSingleObject(pi.hProcess, INFINITE);

    rotateLog(false);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    delete[] cmdline;
    return 0;
}