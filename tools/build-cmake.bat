@echo off
rem Runs CMake to configure GEOS for Visual Studio 2017.
rem Runs MSBuild to build the generated solution.
rem
rem Usage:
rem 1. Copy build.bat to build.local.bat (git ignored file)
rem 2. Make your adjustments in the CONFIGURATION section below
rem 3. Run build.local.bat 32|64
rem 4. Optionally, run devenv.exe {project}{32|64}.sln from command line

rem ### CONFIGURATION #####################################
rem #######################################################

if [%1]==[] goto :Usage
if [%1]==[32] goto :32
if [%1]==[64] goto :64
goto :Usage

:32
set GEOSP=32
set MSBUILDP=Win32
set GENERATOR="Visual Studio 15 2017"
goto :Build

:64
set GEOSP=64
set MSBUILDP=x64
set GENERATOR="Visual Studio 15 2017 Win64"
goto :Build

:Build
set BUILDDIR=_build.vs2017x%GEOSP%
mkdir %BUILDDIR%
pushd %BUILDDIR%
"C:\Program Files\CMake\bin\cmake.exe" ^
    -G %GENERATOR% ^
    ..
move geos.sln geos%GEOSP%.sln
REM msbuild.exe geos%GEOSP%.sln /p:Configuration=Release /p:Platform=%MSBUILDP%
popd
goto :EOF

:Usage
@echo build.bat
@echo Usage: build.bat [32 or 64]
exit /B 1
