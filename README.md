**Disclaimer
This project is purely educational and meant for learning purposes only. It should be used responsibly and legally, respecting privacy and consent laws in your jurisdiction. The author do not condone or encourage any unauthorized use or distribution of this software.**


**Keylogger and Anti-Keylogger Project**
-_This project consists of a basic keylogger and an anti-keylogger implemented in C++ for educational purposes._

**1. Keylogger**
-It does low-level capturing of keystrokes using Windows hooks. This keylogger logs keystrokes into a file called data.txt and enables itself to run at system boot by adding that to the registry.

**Features:**
-Captures keystrokes using a low-level keyboard hook.
-Logs keystrokes to data.txt in the application directory.
-Runs in stealth mode to hide its presence.

**Usage:**
-Run the executable (keylogger.exe).
-Keystrokes will be logged to data.txt in the same directory.

**2. Anti-Keylogger**
-The anti-keylogger component is designed to detect and mitigate the presence of the keylogger described above. It scans for suspicious files, registry entries, and terminates the keylogger process if detected.

**Features:**
-Detects keyloggers by scanning running processes and registry entries.
-Terminates the keylogger process if detected.
-Deletes suspicious files and removes registry entries associated with the keylogger.

**Usage:**
- Run the executable (anti-keylogger.exe).
- It will perform checks to detect the keylogger.
- If a keylogger is detected, it will terminate its process and delete associated files and registry entries.



**Instructions**
Building from Source:

- Clone the repository: git clone https://github.com/gazam0010/keylogger.git
- Open the project in Visual Studio 2022 (preferrable) or your preferred IDE.
- Build the keylogger and anti-keylogger projects.
- Running the Applications:

Ensure you have administrative privileges to run both applications effectively.
- Execute keylogger.exe to start capturing keystrokes.
- Execute anti-keylogger.exe to detect and mitigate the keylogger.
  
**Contributing:**

Contributions are welcome! Fork the repository and create a pull request with your changes.
