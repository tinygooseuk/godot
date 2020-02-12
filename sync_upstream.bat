@echo off

git pull upstream 3.2
if %errorlevel%==1 goto error

git push origin 3.2

exit /B

error:
echo There was an error :(