#!/usr/bin/bash

git pull upstream 3.2
retVal=$?
if [ $retVal -ne 0 ]; then
	echo "Error pulling upstream"
	exit $retVal	
fi


git push origin 3.2
retVal=$?
if [ $retVal -ne 0 ]; then
	echo "Error pushing to master"
	exit $retVal	
fi
