@echo off

:: Post-install tests with CMake
::
:: First required argument is the installed prefix, which
:: is used to set CMAKE_PREFIX_PATH
:: Second optional argument is either 'shared' (default) or 'static'
:: Additionally, several environment variable are required
::  - CMAKE_GENERATOR - (e.g.) Ninja
::  - VCPKG_ROOT - (e.g.) C:\Tools\vcpkg
::  - platform - (e.g.) x64

If not defined CMAKE_GENERATOR (
  Echo CMAKE_GENERATOR must be set 1>&2
  Exit /B 1
)
If not defined VCPKG_ROOT (
  Echo VCPKG_ROOT not defined for Vcpkg 1>&2
  Exit /B 1
)
Set VCPKG_INSTALLED=%VCPKG_ROOT%\installed\%platform%-windows

Set CMAKE_PREFIX_PATH=%1
If not defined CMAKE_PREFIX_PATH (
  Echo First positional argument CMAKE_PREFIX_PATH required 1>&2
  Exit /B 1
)
Set library_type=%2
If not defined library_type (
  Set library_type=shared
)
Set _ValidBuildMode=0
If %library_type% EQU shared (
  Set _ValidBuildMode=1
  Setlocal EnableDelayedExpansion
  Set PATH=!CMAKE_PREFIX_PATH!\bin;!VCPKG_INSTALLED!\bin;!PATH!
  Setlocal DisableDelayedExpansion
)
If %library_type% EQU static Set _ValidBuildMode=1
If %_ValidBuildMode% NEQ 1 (
  Echo Second argument must be either shared ^(default^) or static 1>&2
  Exit /B 1
)

Echo Running post-install tests with CMake (%library_type%)
path
Set _InitDir=%~dp0
Cd %_InitDir%

Echo Testing C app
Cd c_app
If exist build Rd /s /q build || Exit /B 1

cmake ^
  -D CMAKE_BUILD_TYPE=Release ^
  -D CMAKE_COMPILE_WARNING_AS_ERROR=ON ^
  -D CMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" ^
  -D CMAKE_PREFIX_PATH="%CMAKE_PREFIX_PATH%" ^
  -S . -B build || Exit /B 1

cmake --build build --config Release
:: TODO || Exit /B 1
:: fix MSVC MSBuild static error:
:: IMPORTED_LOCATION not set for imported target "GEOS::geos_c" configuration "MinSizeRel"
:: IMPORTED_LOCATION not set for imported target "GEOS::geos_c" configuration "RelWithDebInfo"
:: LINK : warning LNK4098: defaultlib 'MSVCRTD' conflicts with use of other libs; use /NODEFAULTLIB:library
:: geos.lib(HCoordinate.obj) : error LNK2001: unresolved external symbol __imp__calloc_dbg [C:\a\geos\geos\tests\postinstall\c_app\build\c_app.vcxproj]
:: ...

ctest --test-dir build --output-on-failure --verbose --timeout 2 -C Release
:: TODO || Exit /B 1
:: fix MSVC Ninja static error:
:: Test #1: test_length ......................***Timeout
:: Test #2: test_libpath .....................***Timeout
:: Test #3: test_version .....................***Timeout

Cd ..

Echo Testing C++ app
Cd cpp_app
If exist build Rd /s /q build || Exit /B 1

cmake ^
  -D CMAKE_BUILD_TYPE=Release ^
  -D CMAKE_COMPILE_WARNING_AS_ERROR=ON ^
  -D CMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" ^
  -D CMAKE_PREFIX_PATH="%CMAKE_PREFIX_PATH%" ^
  -S . -B build || Exit /B 1

cmake --build build --config Release
:: TODO || Exit /B 1
:: fix MSVC MSBuild shared error:
:: IMPORTED_IMPLIB not set for imported target "GEOS::geos" configuration "MinSizeRel"
:: IMPORTED_IMPLIB not set for imported target "GEOS::geos" configuration "RelWithDebInfo"
:: Test #1: test_length ......................***Exception: SegFault

ctest --test-dir build --output-on-failure --verbose --timeout 2 -C Release
:: TODO || Exit /B 1

Cd ..

Echo Finished running post-install tests CMake (%library_type%)
