#!/bin/bash

# Android SDK
if [[ `uname -s` == Linux* ]]; then
	export ANDROID_HOME=$TRAKTOR_HOME/3rdp/android-sdk/linux
elif [[ `uname -s` == Darwin* ]]; then
	export ANDROID_HOME=$TRAKTOR_HOME/3rdp/android-sdk/darwin
fi

# Android NDK
export ANDROID_NDK_ROOT=$TRAKTOR_HOME/3rdp/android-ndk-r21
