#include <windows.h>
#include <iostream>
#include <fstream>
#include <psapi.h>
#include <TlHelp32.h>
#include <string>
#include <sstream>
#include <vector>
#include <filesystem>
#include <map>
#include <algorithm>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "advapi32.lib")

#define MAX_KEY_LENGTH 255  
TCHAR achKey[MAX_KEY_LENGTH];

    

std::vector<std::string> ReadSignaturesFromFile(const std::string& filename) {
    std::vector<std::string> signatures;
    std::ifstream file(filename);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            signatures.push_back(line);
        }
        file.close();
    }
    return signatures;
}

bool DetectLowLevelKeyboardHook(const std::vector<std::string>& signatures) {
    HMODULE hMods[1024];
    HANDLE hProcess;
    DWORD cbNeeded;
    unsigned int i;

    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetCurrentProcessId());
    if (NULL == hProcess)
        return false;

    bool keyloggerDetected = false;

    if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
        for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
            TCHAR szModName[MAX_PATH];
            if (GetModuleFileNameEx(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR))) {
                std::wstring ws(szModName);
                std::string moduleName(ws.begin(), ws.end());
                for (const auto& signature : signatures) {
                    if (moduleName.find(signature) != std::string::npos) {
                        keyloggerDetected = true;
                        break;
                    }
                }
                if (keyloggerDetected)
                    break;
            }
        }
    }
    CloseHandle(hProcess);
    return keyloggerDetected;
}

bool DetectSuspiciousRegistryEntry(const std::vector<std::string>& signatures, std::wstring& keyloggerFilePath) {
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        TCHAR achValue[MAX_KEY_LENGTH];
        DWORD cchValue = MAX_KEY_LENGTH;
        DWORD valueType;
        DWORD cbData;

        LONG retCode = RegQueryValueEx(hKey, L"MyKL", NULL, &valueType, (LPBYTE)achValue, &cchValue);
        if (retCode == ERROR_SUCCESS) {
            if (valueType == REG_SZ) {
                keyloggerFilePath = std::wstring(achValue);
                RegCloseKey(hKey);
                return true;
            }
        }
        RegCloseKey(hKey);
    }
    return false;
}

void TerminateKeyloggerProcess(const std::vector<std::string>& signatures, const std::wstring& keyloggerFilePath) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe)) {
        do {
            std::wstring ws(pe.szExeFile);
            std::string processName(ws.begin(), ws.end());
            std::transform(processName.begin(), processName.end(), processName.begin(), ::tolower);

            for (const auto& signature : signatures) {
                if (processName.find(signature) != std::string::npos) {
                    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
                    if (hProcess) {
                        TerminateProcess(hProcess, 1);
                        CloseHandle(hProcess);

                        // Display message for terminated process
                        std::wstring message = L"Process '" + ws + L"' suspected as a keylogger.\nThe following action has been taken:\nKilled the process, deleted the suspicious file, and removed the registry entry.";
                        MessageBox(NULL, message.c_str(), L"Process Terminated", MB_OK | MB_ICONINFORMATION);

                        // Delete keylogger file
                        fs::path keyloggerPath(keyloggerFilePath);
                        if (fs::exists(keyloggerPath) && fs::is_regular_file(keyloggerPath)) {
                            fs::remove(keyloggerPath);
                        }

                        // Delete registry entry
                        HKEY hKey;
                        if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
                            RegDeleteValue(hKey, L"MyKL");
                            RegCloseKey(hKey);
                        }
                    }
                    break;
                }
            }
        } while (Process32Next(hSnapshot, &pe));
    }
    CloseHandle(hSnapshot);
}

void ShowAlert() {
    MessageBox(NULL, L"Keylogger detected!", L"Security Alert", MB_OK | MB_ICONWARNING);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    bool keyloggerDetected = false;
    std::wstring keyloggerFilePath;

    // Read signatures from file
    std::vector<std::string> signatures = ReadSignaturesFromFile("keylogger_signatures.txt");

    // Perform detection checks
    if (DetectLowLevelKeyboardHook(signatures)) {
        keyloggerDetected = true;
    }

    if (DetectSuspiciousRegistryEntry(signatures, keyloggerFilePath)) {
        keyloggerDetected = true;
    }

    // Show alert and take actions if keylogger detected
    if (keyloggerDetected) {
        ShowAlert();
        TerminateKeyloggerProcess(signatures, keyloggerFilePath);
    }
    else {
        MessageBox(NULL, L"No keylogger detected.", L"System Secure", MB_OK | MB_ICONINFORMATION);
    }

    return 0;
}
