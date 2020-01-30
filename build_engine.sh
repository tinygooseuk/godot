#!/usr/bin/bash

echo **************************************************
echo Building vanilla editor with mono module...
echo **************************************************

scons p=x11 use_llvm=yes tools=yes module_mono_enabled=yes mono_glue=no -j16
retVal=$?
if [ $retVal -ne 0 ]; then
	echo "Error building vanilla editor!"
	exit $retVal	
fi

echo **************************************************
echo Generating Mono type data...
echo **************************************************

bin/godot.x11.tools.64.llvm.mono --generate-mono-glue modules/mono/glue 
retVal=$?
if [ $retVal -ne 0 ]; then
	echo "Error generated glue code!"
	exit $retVal	
fi

echo **************************************************
echo Building Godot Engine...
echo **************************************************

scons p=x11 use_llvm=yes tools=yes module_mono_enabled=yes mono_glue=yes -j16
retVal=$?
if [ $retVal -ne 0 ]; then
	echo "Error building final editor binary!"
	exit $retVal	
fi
