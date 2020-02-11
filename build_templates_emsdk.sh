#!/usr/bin/bash

VERSION=4.0.dev.mono

echo **************************************************
echo Preparing to build export templates for Win64...
echo **************************************************

rm -rf ~/.godot/templates/$VERSION

echo **************************************************
echo Building DEBUG template for Emscripten...
echo **************************************************
scons platform=javascript tools=no module_mono_enabled=yes mono_glue=yes mono_prefix=/etc/mono target=release_debug -j16

retVal=$?
if [ $retVal -ne 0 ]; then
	echo "Error building DEBUG"
	exit $retVal	
fi


cp bin/godot.javascript.opt.debug.zip "~/.local/shared/godot/templates/$VERSION/webassembly_debug.zip"


echo **************************************************
echo Building RELEASE template for Emscripten...
echo **************************************************

scons platform=javascript tools=no module_mono_enabled=yes mono_glue=yes mono_prefix=/etc/mono target=release -j16
retVal=$?
if [ $retVal -ne 0 ]; then
	echo "Error building RELEASE"
	exit $retVal	
fi

cp bin/godot.javascript.opt.zip "~/.local/shared/godot/templates/$VERSION/webassembly_release.zip"