@echo on

:: Post-install tests with CMake
::
:: First required argument is the installed prefix, which
:: is used to set CMAKE_PREFIX_PATH


echo Running post-install tests with CMake

set CMAKE_PREFIX_PATH=%1
if not defined CMAKE_PREFIX_PATH (
    echo First positional argument CMAKE_PREFIX_PATH required
    exit /B 1
)

echo CMAKE_PREFIX_PATH=%CMAKE_PREFIX_PATH%

cd %~dp0

cd test_c
del /f /q build 2> nul
md build
cd build
cmake -G"%GENERATOR%" -DCMAKE_BUILD_TYPE=%CONFIGURATION% -DCMAKE_PREFIX_PATH=%CMAKE_PREFIX_PATH% .. || exit /B 2
cmake --build . --config %CONFIGURATION% || exit /B 3
ctest --output-on-failure -C %CONFIGURATION% || exit /B 4
cd ..
del /f /q build
cd ..

cd test_cpp
del /f /q build 2> nul
md build
cd build
cmake -G"%GENERATOR%" -DCMAKE_BUILD_TYPE=%CONFIGURATION% -DCMAKE_PREFIX_PATH=%CMAKE_PREFIX_PATH% .. || exit /B 2
cmake --build . --config %CONFIGURATION% || exit /B 3
ctest --output-on-failure -C %CONFIGURATION% || exit /B 4
cd ..
del /f /q build

cd ..
