import psutil

# Heuristic checks for suspicious process names or high CPU usage
suspicious_keywords = ['keylogger', 'malware', 'spy']

def check_suspicious_processes():
    for proc in psutil.process_iter(['pid', 'name', 'exe', 'cmdline', 'cpu_percent']):
        try:
            process_info = proc.info
            if any(keyword in process_info['name'].lower() for keyword in suspicious_keywords):
                print(f"Suspicious process detected: {process_info['name']} (PID: {process_info['pid']})")
            if process_info['cpu_percent'] > 50:
                print(f"High CPU usage detected: {process_info['name']} (PID: {process_info['pid']}) CPU: {process_info['cpu_percent']}%")
        except (psutil.NoSuchProcess, psutil.AccessDenied):
            continue

if __name__ == "__main__":
    check_suspicious_processes()
