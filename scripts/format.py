import os
import subprocess


def format_files(directory):
    for root, dirs, files in os.walk(directory):
        if 'vcpkg' in dirs:
            # Exclude the 'vcpkg' directory from traversal
            dirs.remove('vcpkg')
        if 'emsdk' in dirs:
            # Exclude the 'emsdk' directory from traversal
            dirs.remove('emsdk')
        if 'lib' in dirs:
            dirs.remove('lib')  # Exclude the third party libs from traversal

        for file in files:
            if any(file.endswith(extension) for extension in ('.cpp', '.hpp', '.cc', '.cxx', '.h', '.c')):
                file_path = os.path.join(root, file)
                subprocess.run(['clang-format', '-style=file',
                               '-i', file_path], check=True)


# Usage
format_files('.')
