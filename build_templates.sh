#!/usr/bin/bash

VERSION=3.2.1.rc.mono

echo **************************************************
echo Preparing to build export templates for X11 Linux...
echo **************************************************

rm -rf ~/.godot/templates/$VERSION

echo **************************************************
echo Building DEBUG template for X11 Linux...
echo **************************************************

scons p=x11 use_llvm=yes tools=no module_mono_enabled=yes mono_glue=yes target=release_debug bits=64 -j16
retVal=$?
if [ $retVal -ne 0 ]; then
	echo "Error building DEBUG"
	exit $retVal	
fi
cp bin/godot.x11.opt.debug.64.llvm.mono "~/.local/shared/godot/templates/$VERSION/linux_x11_64_debug"
cp -R bin/data.mono.x11.64.release_debug "~/.local/shared/godot/templates/$VERSION/data.mono.x11.64.release_debug"



echo **************************************************
echo Building RELEASE template for X11 Linux...
echo **************************************************

scons p=x11 use_llvm=yes tools=no module_mono_enabled=yes mono_glue=yes target=release bits=64 -j16
retVal=$?
if [ $retVal -ne 0 ]; then
	echo "Error building RELEASE"
	exit $retVal	
fi
cp bin/godot.x11.opt.64.llvm.mono "~/.local/shared/godot/templates/$VERSION/linux_x11_64_release"
cp -R bin/data.mono.x11.64.release "~/.local/shared/godot/templates/$VERSION/data.mono.x11.64.release"
