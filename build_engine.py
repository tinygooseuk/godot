#!/usr/bin/env python3
import sys
import os
import shutil

##### CONSTS #########################
VERSION = "3.2.1.rc.mono"



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

"linux_x11_64_debug"
"windows_64_debug"

def get_godot_debug_template_exe_name():
    if os.name == "posix":
        return "bin/godot.x11.opt.debug.64.llvm.mono"
    else:
        return "bin\\godot.windows.opt.debug.64.mono.exe"

def get_godot_release_template_exe_name():
    if os.name == "posix":
        return "bin/godot.x11.opt.64.llvm.mono"
    else:
        return "bin\\godot.windows.opt.64.mono.exe"

def get_platform_specific_build_args():
    if os.name == "posix":
        return f"p={get_godot_platform_name()} use_llvm=yes"
    else:
        return f"p={get_godot_platform_name()}"

def get_platform_godot_appdata_dir():
    if os.name == "posix":
        return f"{os.getenv("HOME")}/.local/share/godot"
    else:
        return f"{os.getenv("APPDATA")}\\Godot"

def run(command):
    return os.system(command) == 0

##### TEMPLATE HELPER FUNCS #########################
def get_current_template_dir():
    return get_platform_godot_appdata_dir() + os.pathsep + "templates" + os.pathsep + VERSION


##### BUILD STEPS #########################
def build_initial_engine():
    if not run(f"scons {get_platform_specific_build_args()} tools=yes module_mono_enabled=yes mono_glue=no bits=64 -j16"):
        print("Error building vanilla error!")
        exit(1)

def build_glue():
    if not run(f"{get_godot_tools_exe_name()} --generate-mono-glue modules/mono/glue"):
        print("Error generating glue code!")
        exit(1)

def build_final_engine():
    if not run(f"scons {get_platform_specific_build_args()} tools=yes module_mono_enabled=yes mono_glue=yes bits=64 -j16"):
        print("Error building final editor!")
        exit(1)



##### BUILD TEMPLATES #########################
def build_templates():
    # Delete and re-create the template dir
    template_dir = get_current_template_dir()
    shutil.rmtree(template_dir)

    build_template_debug()
    build_template_release()



def build_template_debug():
    if not run(f"scons {get_platform_specific_build_args()}} tools=no module_mono_enabled=yes mono_glue=yes target=release_debug bits=64 -j16"):
        print("Error building DEBUG template!")
        exit(1)

    #cp bin/godot.x11.opt.debug.64.llvm.mono "$HOME/.local/share/godot/templates/$VERSION"
    #mv "$HOME/.local/share/godot/templates/$VERSION/godot.x11.opt.debug.64.llvm.mono" "$HOME/.local/share/godot/templates/$VERSION/linux_x11_64_debug"
    #cp -R bin/data.mono.x11.64.release_debug "$HOME/.local/share/godot/templates/$VERSION/data.mono.x11.64.release_debug"

    #copy bin\godot.windows.opt.debug.64.mono.exe "%APPDATA%\Godot\templates\%VERSION%\windows_64_debug.exe"
    #robocopy bin\data.mono.windows.64.release_debug "%APPDATA%\Godot\templates\%VERSION%\data.mono.windows.64.release_debug" /E /is /it



def build_template_release():
    if not run(f"scons {get_platform_specific_build_args()}} tools=no module_mono_enabled=yes mono_glue=yes target=release bits=64 -j16"):
        print("Error building DEBUG template!")
        exit(1)
    
    #cp bin/godot.x11.opt.64.llvm.mono "$HOME/.local/share/godot/templates/$VERSION"
    #mv "$HOME/.local/share/godot/templates/$VERSION/godot.x11.opt.64.llvm.mono" "$HOME/.local/share/godot/templates/$VERSION/linux_x11_64_release"
    #cp -R bin/data.mono.x11.64.release "$HOME/.local/share/godot/templates/$VERSION"

    #copy bin\godot.windows.opt.64.mono.exe "%APPDATA%\Godot\templates\%VERSION%\windows_64_release.exe"
    #robocopy bin\data.mono.windows.64.release "%APPDATA%\Godot\templates\%VERSION%\data.mono.windows.64.release" /E /is /it



##### ENTRY POINT #########################
def main(action):
    if action == "full":
        build_initial_engine()
        build_glue()
        build_final_engine()
    elif action == "glue":
        build_glue()
        build_final_engine()
    elif action == "quick":
        build_final_engine()
    elif action == "templates":
        build_templates()
    else:
        usage()

if __name__ == "__main__":
    action = "glue"

    if len(sys.argv) > 1:
        action = sys.argv[1]
        
    main(action)

    if os.name == "posix":
        input()