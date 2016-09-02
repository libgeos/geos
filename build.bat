@echo off
rem Runs CMake to configure for Visual Studio 2015.
rem Runs MSBuild to build the generated solution.
rem
rem Usage:
rem 1. Copy build.bat to build.locale.bat (git ignored file)
rem 2. Make your adjustments in the CONFIGURATION section below
rem 3. Run build.local.bat 32|64
rem 4. Optionally, run devenv.exe GEOS{32|64}.sln

rem ### CONFIGURATION #####################################
rem #######################################################

if not defined VS140COMNTOOLS goto :NoVS
if [%1]==[] goto :Usage
if [%1]==[32] goto :32
if [%1]==[64] goto :64
goto :Usage

:32
set BUILDP=32
set MSBUILDP=Win32
set GENERATOR="Visual Studio 14 2015"
goto :Build

:64
set BUILDP=64
set MSBUILDP=x64
set GENERATOR="Visual Studio 14 2015 Win64"
goto :Build

:Build
set BUILDDIR=_build%BUILDP%
mkdir %BUILDDIR%
pushd %BUILDDIR%
"C:\Program Files\CMake\bin\cmake.exe" ^
    -G %GENERATOR% ^
    ..
move GEOS.sln GEOS%BUILDP%.sln
msbuild.exe GEOS%BUILDP%.sln /p:Configuration=Release /p:Platform=%MSBUILDP%
popd
goto :EOF

:NoVS
@echo build.bat
@echo  Visual Studio 2015 not found
@echo  "%%VS140COMNTOOLS%%" environment variable not defined
exit /B 1

:Usage
@echo build.bat
@echo Usage: build.bat [32 or 64]
exit /B 1
