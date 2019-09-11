@echo off

scons p=windows vcproj=yes -j8
if %errorlevel%==1 goto error

exit /B

error:
echo There was an error!