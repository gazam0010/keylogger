import hashlib
import os

# Known keylogger hash signatures
known_hashes = {
    '5d41402abc4b2a76b9719d911017c592': 'keylogger.exe', # Example hash
    # Add more hashes as needed
}

def get_file_hash(filepath):
    hasher = hashlib.md5()
    with open(filepath, 'rb') as file:
        buf = file.read()
        hasher.update(buf)
    return hasher.hexdigest()

def scan_directory(directory):
    for root, _, files in os.walk(directory):
        for file in files:
            filepath = os.path.join(root, file)
            file_hash = get_file_hash(filepath)
            if file_hash in known_hashes:
                print(f"Suspicious file detected: {filepath} (Hash: {file_hash})")

if __name__ == "__main__":
    scan_directory("C:\\")  # Replace with the directory you want to scan
