import os
import subprocess

def format_files(directory):
    for root, dirs, files in os.walk(directory):
        if 'vcpkg' in dirs:
            dirs.remove('vcpkg')  # Exclude the 'vcpkg' directory from traversal
        if 'emsdk' in dirs:
            dirs.remove('emsdk')  # Exclude the 'emsdk' directory from traversal
        
        for file in files:
            if any(file.endswith(extension) for extension in ('.cpp', '.hpp', '.cc', '.cxx', '.h', '.c')):
                file_path = os.path.join(root, file)
                subprocess.run(['clang-format', '-style=file', '-i', file_path], check=True)

# Usage
format_files('.')
