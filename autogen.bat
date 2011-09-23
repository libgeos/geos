@ECHO OFF
REM
REM This script generates headers for use with Visual C++ only
REM
REM Usage:
REM .\autogen.bat
REM nmake -f makefile.vc
REM
REM NOTE: DO NOT include this script in GEOS sources distribution
REM

set GEOS_HEADERS=include\geos

COPY %GEOS_HEADERS%\version.h.vc %GEOS_HEADERS%\version.h 
COPY %GEOS_HEADERS%\platform.h.vc %GEOS_HEADERS%\platform.h
COPY capi\geos_c.h.in capi\geos_c.h
