@echo off

git pull upstream master
if %errorlevel%==1 goto error

git push origin master

exit

error:
echo There was an error :(