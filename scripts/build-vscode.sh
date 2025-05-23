#!/bin/bash

# Ensure terminal output is cleared.
clear

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/config.sh"

if [[ "$1" == "Linux" ]]; then

	pushd $TRAKTOR_HOME/build/linux
	make -s -j -f 'Extern Linux.mak' $2
	make -s -j -f 'Traktor Linux.mak' $2
	popd

elif [[ "$1" == "macOS" ]]; then

	pushd $TRAKTOR_HOME/build/osx
	make -s -j -f 'Extern OSX.mak' $2
	make -s -j -f 'Traktor OSX.mak' $2
	popd

elif [[ "$1" == "Android" ]]; then

	pushd $TRAKTOR_HOME/build/android
	make -s -j -f 'Extern Android.mak' $2
	make -s -j -f 'Traktor Android.mak' $2
	popd

elif [[ "$1" == "iOS" ]]; then

	pushd $TRAKTOR_HOME/build/ios
	make -s -j -f 'Extern iOS.mak' $2
	make -s -j -f 'Traktor iOS.mak' $2
	popd

elif [[ "$1" == "RPi" ]]; then

	pushd $TRAKTOR_HOME/build/rpi
	make -s -j -f 'Extern Raspberry Pi.mak' $2
	make -s -j -f 'Traktor Raspberry Pi.mak' $2
	popd

else

	echo "Invalid argument $1"

fi