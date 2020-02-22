#!/usr/bin/env python3
import sys
import os
import shutil

##### CONSTS #########################
DEFAULT_COMMAND = "show"



##### GENERIC HELPER FUNCS #########################
def usage():
    print("Usage:", __file__, "{show|sync}")

def print_banner(words):
    print("**************************************************")
    print(words)
    print("**************************************************")



def fetch_upstream():
    return run("git fetch upstream")

def show_upstream_diff():
    return run("git diff --name-only upstream/3.2")

def merge_upstream_and_push():
    if not run("git pull upstream 3.2"):
        return False

    return run("git push origin 3.2")

def run(command):
    return os.system(command) == 0


##### BUILD STEPS #########################
def upstream_show():
    print_banner("Showing changes from upstream...")

    fetch_upstream()
    show_upstream_diff()

def upstream_sync():
    print_banner("Syncing to upstream...")

    fetch_upstream()
    merge_upstream_and_push()



##### ENTRY POINT #########################
def main(command):
    if command == "show":
        upstream_show()
    elif command == "sync":
        upstream_sync()
    else:
        print("Unknown command: ", command)
        usage()

if __name__ == "__main__":
    if len(sys.argv) == 1:
        print_banner("Command: " + DEFAULT_COMMAND.upper())
        main(DEFAULT_COMMAND)

    for i in range(1, len(sys.argv)):
        print_banner("Command: " + sys.argv[i].upper())
        main(sys.argv[i])
