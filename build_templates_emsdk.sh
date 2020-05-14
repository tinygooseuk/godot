#!/usr/bin/bash

VERSION=3.2.2.rc.mono

echo **************************************************
echo Preparing to build export templates for Emscripten...
echo **************************************************

rm -rf ~/.godot/templates/$VERSION

# Install the Emscripten SDK
git clone --depth=1 "https://github.com/juj/emsdk.git" "emsdk"
cd emsdk
./emsdk install latest
./emsdk activate latest
export EMSCRIPTEN_ROOT
EMSCRIPTEN_ROOT="$(em-config EMSCRIPTEN_ROOT || true)"
cd ..

# Build HTML5 export template
scons platform=javascript tools=no target="$scons_target" \
      "${SCONS_FLAGS[@]}"

# Move HTML5 export template to the artifacts directory
mv "$GODOT_DIR/bin"/godot.javascript.*.zip \
    "$ARTIFACTS_DIR/templates/webassembly_$target.zip"

echo **************************************************
echo Building DEBUG template for Emscripten...
echo **************************************************
scons platform=javascript tools=no module_mono_enabled=yes mono_glue=yes target=release_debug -j16

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
