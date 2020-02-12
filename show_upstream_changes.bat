@echo off

git fetch upstream
if %errorlevel%==1 goto error

git diff --name-only upstream/3.2

exit /B

error:
echo There was an error :(