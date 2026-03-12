from ftplib import FTP
import os
import sys

consoleIP = sys.argv[1]
if '.' not in consoleIP:
    print(sys.argv[0], "ERROR: Please specify with `IP=[Your console's IP]`")
    sys.exit(-1)

consolePort = 5000

deployDir = os.path.join(os.curdir, 'deploy', 'atmosphere')

if not os.path.isdir(deployDir):
    print("ERROR: deploy/atmosphere not found. Run `make` first.")
    sys.exit(-1)


def listdirs(connection, _path):
    file_list, dirs, nondirs = [], [], []
    try:
        connection.cwd(_path)
    except:
        return []

    connection.retrlines('LIST', lambda x: file_list.append(x.split()))
    for info in file_list:
        ls_type, name = info[0], info[-1]
        if ls_type.startswith('d'):
            dirs.append(name)
        else:
            nondirs.append(name)
    return dirs


def ensuredirectory(connection, root, path):
    if path not in listdirs(connection, root):
        connection.mkd(f'{root}/{path}')


def upload_tree(connection, local_root, remote_root):
    for dirpath, dirnames, filenames in os.walk(local_root):
        rel = os.path.relpath(dirpath, local_root).replace('\\', '/')
        if rel == '.':
            remote_dir = remote_root
        else:
            remote_dir = f'{remote_root}/{rel}'

        # Ensure all directories exist
        parts = remote_dir.strip('/').split('/')
        current = ''
        for part in parts:
            parent = current if current else ''
            ensuredirectory(connection, f'/{parent}' if parent else '', part)
            current = f'{current}/{part}' if current else part

        for filename in filenames:
            local_path = os.path.join(dirpath, filename)
            remote_path = f'/{remote_dir}/{filename}'
            print(f'Sending {remote_path}')
            with open(local_path, 'rb') as f:
                connection.storbinary(f'STOR {remote_path}', f)


ftp = FTP()
print(f'Connecting to {consoleIP}... ', end='')
ftp.connect(consoleIP, consolePort)
print('Connected!')

upload_tree(ftp, deployDir, 'atmosphere')

print('Done!')
