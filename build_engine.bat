@echo off

echo **************************************************
echo Building vanilla editor with mono module...
echo **************************************************

call scons p=windows tools=yes module_mono_enabled=yes mono_glue=no -j16
if %errorlevel%==1 goto error



echo **************************************************
echo Generating Mono type data...
echo **************************************************

call bin\godot.windows.tools.64.mono.exe --generate-mono-glue modules/mono/glue 
if %errorlevel%==1 goto error



echo **************************************************
echo Building Godot Engine...
echo **************************************************

call scons p=windows tools=yes module_mono_enabled=yes mono_glue=yes vsproj=yes -j16




if %errorlevel%==1 goto error
exit /B

error:
echo There was an error!