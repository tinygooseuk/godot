@echo off

set VERSION=3.2.1.rc.mono

echo **************************************************
echo Preparing to build export templates for Win64...
echo **************************************************

rd /s/q "%APPDATA%\Godot\templates\%VERSION%"
mkdir "%APPDATA%\Godot\templates\%VERSION%"



echo **************************************************
echo Building DEBUG template for Win64...
echo **************************************************

call scons p=windows tools=no module_mono_enabled=yes mono_glue=yes target=release_debug bits=64 -j16
copy bin\godot.windows.opt.debug.64.mono.exe "%APPDATA%\Godot\templates\%VERSION%\windows_64_debug.exe"
robocopy bin\data.mono.windows.64.release_debug "%APPDATA%\Godot\templates\%VERSION%\data.mono.windows.64.release_debug" /E /is /it



echo **************************************************
echo Building RELEASE template for Win64...
echo **************************************************

call scons p=windows tools=no module_mono_enabled=yes mono_glue=yes target=release bits=64 -j16
copy bin\godot.windows.opt.64.mono.exe "%APPDATA%\Godot\templates\%VERSION%\windows_64_release.exe"
robocopy bin\data.mono.windows.64.release "%APPDATA%\Godot\templates\%VERSION%\data.mono.windows.64.release" /E /is /it