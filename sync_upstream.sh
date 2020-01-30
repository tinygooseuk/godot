#!/usr/bin/bash

git pull upstream master
retVal=$?
if [ $retVal -ne 0 ]; then
	echo "Error pulling upstream"
	exit $retVal	
fi


git push origin master
retVal=$?
if [ $retVal -ne 0 ]; then
	echo "Error pushing to master"
	exit $retVal	
fi
