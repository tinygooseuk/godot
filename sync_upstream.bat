@echo off

git pull upstream master
if %errorlevel%==1 goto error

git push origin master

exit /B

error:
echo There was an error :(