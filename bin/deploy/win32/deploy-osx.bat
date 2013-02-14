@echo off

set BUNDLE=%DEPLOY_PROJECT_NAME: =%

if "%1"=="build" (

	%DEPLOY_PROJECT_ROOT%\bin\latest\win32\releaseshared\Traktor.Pipeline.App -p -s=Pipeline -l=Pipeline.log %2 %3 %4 %5

) else if "%1"=="deploy" (

	rem Build application bundle.
	mkdir -p "%BUNDLE%.app\Contents\Resources"

	copy /Y %DEPLOY_PROJECT_ROOT:/=\%\res\deploy\osx\Info.plist "%BUNDLE%.app\Contents\" > "%DEPLOY_OUTPUT_PATH%\Deploy.log"
	copy /Y %DEPLOY_PROJECT_ROOT:/=\%\res\deploy\osx\Icon.icns "%BUNDLE%.app\Contents\Resources\" >> "%DEPLOY_OUTPUT_PATH%\Deploy.log"
	copy /Y %DEPLOY_PROJECT_ROOT:/=\%\bin\latest\osx\releaseshared\*.* "%BUNDLE%.app\Contents\MacOS\" >> "%DEPLOY_OUTPUT_PATH%\Deploy.log"
	copy /Y Application.config "%BUNDLE%.app\Contents\Resources\" >> %DEPLOY_OUTPUT_PATH%\Deploy.log

	rem Modify application meta data.
	%TRAKTOR_HOME%\bin\win32\Replace -e=utf8 "%BUNDLE%.app\Contents\Info.plist" "$DEPLOY_PROJECT_NAME" "%DEPLOY_PROJECT_NAME%" "$DEPLOY_EXECUTABLE" "%DEPLOY_EXECUTABLE%" "$DEPLOY_PROJECT_IDENTIFIER" "%DEPLOY_PROJECT_IDENTIFIER%" >> "%DEPLOY_OUTPUT_PATH%\Deploy.log"

	rem Deploy entire application bundle.
	pushd "%BUNDLE%.app"
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy -recursive -target-base="%BUNDLE%.app" %DEPLOY_TARGET_HOST% * >> "%DEPLOY_OUTPUT_PATH%\Deploy.log"
	popd

	rem Deploy launcher script.
	pushd %TRAKTOR_HOME%\bin\win32
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy %DEPLOY_TARGET_HOST% launch-osx.sh >> "%DEPLOY_OUTPUT_PATH%\Deploy.log"
	popd

) else if "%1"=="launch" (

	%TRAKTOR_HOME%\bin\win32\RemoteLaunch %DEPLOY_TARGET_HOST% launch-osx.sh "%BUNDLE%.app" > "%DEPLOY_OUTPUT_PATH%\Launch.log"

) else if "%1"=="migrate" (

	rem Migrate database.
	%DEPLOY_PROJECT_ROOT%\bin\latest\win32\releaseshared\Traktor.Database.Migrate.App -s=Migrate -l=Migrate.log

	rem Build application bundle.
	mkdir -p "%BUNDLE%.app\Contents\Resources"

	copy /Y %DEPLOY_PROJECT_ROOT:/=\%\res\deploy\osx\Info.plist "%BUNDLE%.app\Contents\" >> "%DEPLOY_OUTPUT_PATH%\Migrate.log"
	copy /Y %DEPLOY_PROJECT_ROOT:/=\%\res\deploy\osx\Icon.icns "%BUNDLE%.app\Contents\Resources\" >> "%DEPLOY_OUTPUT_PATH%\Migrate.log"
	copy /Y %DEPLOY_PROJECT_ROOT:/=\%\bin\latest\osx\releaseshared\*.* "%BUNDLE%.app\Contents\MacOS\" >> "%DEPLOY_OUTPUT_PATH%\Migrate.log"
	copy /Y Application.config "%BUNDLE%.app\Contents\Resources\" >> "%DEPLOY_OUTPUT_PATH%\Migrate.log"
	copy /Y Content.compact "%BUNDLE%.app\Contents\Resources\" >> "%DEPLOY_OUTPUT_PATH%\Migrate.log"

	rem Modify application meta data.
	%TRAKTOR_HOME%\bin\win32\Replace -e=utf8 "%BUNDLE%.app\Contents\Info.plist" "$DEPLOY_PROJECT_NAME" "%DEPLOY_PROJECT_NAME%" "$DEPLOY_EXECUTABLE" "%DEPLOY_EXECUTABLE%" "$DEPLOY_PROJECT_IDENTIFIER" "%DEPLOY_PROJECT_IDENTIFIER%" >> "%DEPLOY_OUTPUT_PATH%\Migrate.log"
	
)
