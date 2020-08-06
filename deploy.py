#!/usr/bin/python3
import sys, subprocess, os

# Use the script to create a 'deployment profile' that contains the remote host, credentials and paths, and which
# can then be selected by the user. The profiles are persisted (as data appended to this file) and one
# of those is set as default.

USERNAME="dumitru"
PASSWORD="noviflow"
SOURCE_PATH=os.getcwd()
DEPLOY_PATH="/home/dumitru/work/dp4c"

def stdout_print(text):
    sys.stdout.write(text)
    sys.stdout.flush()

def colored(message, color):
    color_code = ""
    if color == "blue":
        color_code = "34"
    elif color == "red":
        color_code = "31"
    return f"\x1b[01;{color_code}m{message}\x1b[00m"

def main(args):
    if len(args) == 0:
        stdout_print("REMOTE HOST argument is required\n")
        return
    remote_host = args[0]
    stdout_print("deploying to %s:%s\n" % (colored(remote_host, "red"), colored(DEPLOY_PATH, "blue")))
    cybermapper_exclude_patterns = []
    novitest_exclude_patterns = []
    common_exclude_patterns = ["deploy.py", "build.sh", "*.log",
                               ".idea/", ".vimprj/", ".git/", ".gitignore",
                               "__pycache__/", "*.pyc", "*.bak"]
    exclude_patterns = common_exclude_patterns + cybermapper_exclude_patterns + novitest_exclude_patterns
    rsync_parameters = ["--delete"]
    rsync_cmd = f"rsync " + " ".join(rsync_parameters) + \
                " --exclude " + " --exclude ".join(["'" + p + "'" for p in exclude_patterns]) + \
                f" -av {SOURCE_PATH}/ {USERNAME}@{remote_host}:{DEPLOY_PATH}/"
#    stdout_print(rsync_cmd)
    subprocess.call(f"sshpass -p \"{PASSWORD}\" " + rsync_cmd, shell=True)

if __name__ == "__main__":
    args = []
    if len(sys.argv) > 1:
        args = sys.argv[1:]
    main(args)
