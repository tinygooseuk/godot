@echo off

echo Generating Mono Glue build
call scons p=windows tools=yes module_mono_enabled=yes mono_glue=no -j16
if %errorlevel%==1 goto error

echo Generating Mono glue code
call bin\godot.windows.tools.64.mono.exe --generate-mono-glue modules/mono/glue 
if %errorlevel%==1 goto error

echo Building mono editor
call scons p=windows tools=yes module_mono_enabled=yes mono_glue=yes vcproj=yes -j16

if %errorlevel%==1 goto error
exit /B

error:
echo There was an error!