@echo off

call %~dp0../../config.bat

mkdir %~dp0build

%TRAKTOR_HOME%\bin\win32\solutionbuilder -f=msvc -p=$(TRAKTOR_HOME)\bin\msvc-2008-win32.xml RemoteToolsWin32.xms
