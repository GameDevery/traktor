@echo off

call %~dp0config.bat

mkdir %~dp0build

%TRAKTOR_HOME%\bin\win32\solutionbuilder -f=msvc -p=$(TRAKTOR_HOME)\bin\msvc-2012-win32.xml SolutionBuilder.xms
