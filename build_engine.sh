#!/usr/bin/bash

usage() {
	echo "Usage: $0 {full|glue|quick}"
}

build_initial_engine() {
	echo **************************************************
	echo Building vanilla editor with mono module...
	echo **************************************************


	scons p=x11 use_llvm=yes tools=yes module_mono_enabled=yes mono_glue=no -j16
	retVal=$?
	if [ $retVal -ne 0 ]; then
		echo "Error building vanilla editor!"
		exit $retVal	
	fi
}

build_glue() {
	echo **************************************************
	echo Generating Mono type data...
	echo **************************************************

	bin/godot.x11.tools.64.llvm.mono --generate-mono-glue modules/mono/glue 
	retVal=$?
	if [ $retVal -ne 0 ]; then
		echo "Error generated glue code!"
		exit $retVal	
	fi
}

build_final() {
	echo **************************************************
	echo Building Godot Engine...
	echo **************************************************

	scons p=x11 use_llvm=yes tools=yes module_mono_enabled=yes mono_glue=yes -j16
	retVal=$?
	if [ $retVal -ne 0 ]; then
		echo "Error building final editor binary!"
		exit $retVal	
	fi
}

# Check params
if [ $# -eq 0 ]; then
	TYPE=glue
else
	TYPE=$1
fi

case $TYPE in
  "full") 	build_initial_engine; build_glue; build_final ;;
  "glue") 	build_glue; build_final ;;
  "quick") 	build_final ;;
   *) 		usage ;;
esac