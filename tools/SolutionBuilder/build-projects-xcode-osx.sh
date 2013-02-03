#!/bin/sh

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/../../config.sh"

$TRAKTOR_HOME/bin/osx/SolutionBuilder -f=xcode -xcode-root-suffix=-static SolutionBuilderMacOSX.xms -d=DebugStatic -r=ReleaseStatic
