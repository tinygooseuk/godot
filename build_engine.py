#!/usr/bin/env python3
import sys
import os
import shutil

##### CONSTS #########################
DEFAULT_COMMAND = "glue"
VERSION = "3.2.2.rc.mono"



##### GENERIC HELPER FUNCS #########################
def usage():
    print("Usage:", __file__, "{quick|full|glue|templates}")

def print_banner(words):
    print("**************************************************")
    print(words)
    print("**************************************************")



def get_godot_platform_name():
    if os.name == "posix":
        return "x11"
    else:
        return "windows"

def get_godot_tools_exe_name():
    if os.name == "posix":
        return "bin/godot.x11.tools.64.llvm.mono"
    else:
        return "bin\\godot.windows.tools.64.mono.exe"

def get_godot_debug_template_exe_name():
    if os.name == "posix":
        return "bin/godot.x11.opt.debug.64.llvm.mono"
    else:
        return "bin\\godot.windows.opt.debug.64.mono.exe"

def get_godot_debug_data_name():
    if os.name == "posix":
        return "bin/data.mono.x11.64.release_debug"
    else:
        return "bin\\data.mono.windows.64.release_debug"


def get_godot_release_template_exe_name():
    if os.name == "posix":
        return "bin/godot.x11.opt.64.llvm.mono"
    else:
        return "bin\\godot.windows.opt.64.mono.exe"


def get_godot_release_data_name():
    if os.name == "posix":
        return "bin/data.mono.x11.64.release"
    else:
        return "bin\\data.mono.windows.64.release"


def get_godot_template_root_name():
    if os.name == "posix":
        return "linux_x11"
    else:
        return "windows"
        
def get_godot_template_exe_extension():
    if os.name == "posix":
        return ""
    else:
        return ".exe"

def get_platform_specific_build_args():
    if os.name == "posix":
        return f"p={get_godot_platform_name()} use_llvm=yes"
    else:
        return f"p={get_godot_platform_name()}"

def get_platform_godot_appdata_dir():
    if os.name == "posix":
        return f"{os.getenv('HOME')}/.local/share/godot"
    else:
        return f"{os.getenv('APPDATA')}\\Godot"

def run(command):
    return os.system(command) == 0

##### TEMPLATE HELPER FUNCS #########################
def get_current_template_dir():
    return get_platform_godot_appdata_dir() + os.sep + "templates" + os.sep + VERSION


##### BUILD STEPS #########################
def build_initial_engine():
    print_banner("Building Vanilla Engine...")

    if not run(f"scons {get_platform_specific_build_args()} tools=yes module_mono_enabled=yes mono_glue=no bits=64 -j16"):
        print("Error building vanilla error!")
        exit(1)

def build_glue():
    print_banner("Building Mono Glue Code...")

    if not run(f"{get_godot_tools_exe_name()} --generate-mono-glue modules/mono/glue"):
        print("Error generating glue code!")
        exit(1)

def build_final_engine():
    print_banner("Building Final Mono-Enabled Engine...")

    if not run(f"scons {get_platform_specific_build_args()} tools=yes module_mono_enabled=yes mono_glue=yes bits=64 -j16"):
        print("Error building final editor!")
        exit(1)



##### BUILD TEMPLATES #########################
def build_templates():
    # Delete and re-create the template dir
    template_dir = get_current_template_dir()
    shutil.rmtree(template_dir, ignore_errors=True)
    os.mkdir(template_dir)

    build_template_debug()
    build_template_release()



def build_template_debug():
    if not run(f"scons {get_platform_specific_build_args()} tools=no module_mono_enabled=yes mono_glue=yes target=release_debug bits=64 -j16"):
        print("Error building DEBUG template!")
        exit(1)

    shutil.copy(get_godot_debug_template_exe_name(), get_platform_godot_appdata_dir() + os.sep + "templates" + os.sep + VERSION + os.sep + get_godot_template_root_name() + "_64_debug" + get_godot_template_exe_extension());
    shutil.copytree(get_godot_debug_data_name(), get_platform_godot_appdata_dir() + os.sep + "templates" + os.sep + VERSION + os.sep + f"data.mono.{get_godot_platform_name()}.64.release_debug");


def build_template_release():
    if not run(f"scons {get_platform_specific_build_args()} tools=no module_mono_enabled=yes mono_glue=yes target=release bits=64 -j16"):
        print("Error building DEBUG template!")
        exit(1)
    
    shutil.copy(get_godot_release_template_exe_name(), get_platform_godot_appdata_dir() + os.sep + "templates" + os.sep + VERSION + os.sep + get_godot_template_root_name() + "_64_release" + get_godot_template_exe_extension());
    shutil.copytree(get_godot_release_data_name(), get_platform_godot_appdata_dir() + os.sep + "templates" + os.sep + VERSION + os.sep + f"data.mono.{get_godot_platform_name()}.64.release");



##### ENTRY POINT #########################
def main(command):
    if command == "full":
        build_initial_engine()
        build_glue()
        build_final_engine()
    elif command == "glue":
        build_glue()
        build_final_engine()
    elif command == "quick":
        build_final_engine()
    elif command == "templates":
        build_templates()
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
