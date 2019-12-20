@echo off

set VERSION=3.2.beta.mono

echo Building Debug...
call scons p=windows tools=no module_mono_enabled=yes mono_glue=yes target=release_debug bits=64 -j16
copy bin\godot.windows.opt.debug.64.mono.exe %APPDATA%\Godot\templates\%VERSION%\windows_64_debug.exe
robocopy bin\data.mono.windows.64.release_debug %APPDATA%\Godot\templates\%VERSION%\data.mono.windows.64.release_debug /E

echo Building Release...
call scons p=windows tools=no module_mono_enabled=yes mono_glue=yes target=release bits=64 -j16
copy bin\godot.windows.opt.64.mono.exe %APPDATA%\Godot\templates\%VERSION%\windows_64_release.exe
robocopy bin\data.mono.windows.64.release %APPDATA%\Godot\templates\%VERSION%\data.mono.windows.64.release /E