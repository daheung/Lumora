REM Build script for testbed
@ECHO OFF
SetLocal EnableDelayedExpansion

REM Get a list of all the .c files.
SET cFilenames=
FOR /R %%f  IN (*.c) DO (
    SET cFilenames=!cFilenames! "%%f"
)

REM echo "Files:" %cFilenames%

SET assembly=testbed
SET compilerFlags=-g
REM -Wall -Werror
SET includeFlags=-ISource -I../Engine/Source/Core -I../Engine/Source
SET linkerFlags=-L../bin/ -lengine.lib
SET defines=-D_DEBUG -DLUMORA_IMPORT

ECHO "Building %assembly%..."
clang %cFilenames% %compilerFlags% -o ../bin/%assembly%.exe %defines% %includeFlags% %linkerFlags%


@REM REM Build script for Testbed
@REM @ECHO OFF
@REM SetLocal EnableDelayedExpansion

@REM REM Get a list of all the .c and .cpp files.
@REM SET sourceFileNames=

@REM FOR /R %%f IN (*.c) DO (
@REM     SET sourceFileNames=!sourceFileNames! "%%f"
@REM )

@REM FOR /R %%f IN (*.cpp) DO (
@REM     SET sourceFileNames=!sourceFileNames! "%%f"
@REM )

@REM SET assembly=testbed
@REM SET compilerFlags=-g -Wall -Werror
@REM SET includeFlags=-Isrc -I../Engine/Source
@REM SET linkerFlags=-L"..\bin" -lengine
@REM SET defines=-D_DEBUG -DLUMORA_IMPORT

@REM ECHO Building %assembly%...

@REM clang++ %sourceFileNames% %compilerFlags% ^
@REM     -o ../bin/%assembly%.exe ^
@REM     %defines% ^
@REM     %includeFlags% ^
@REM     %linkerFlags%

@REM IF %ERRORLEVEL% NEQ 0 (
@REM     ECHO Build failed.
@REM     EXIT /B %ERRORLEVEL%
@REM )

@REM ECHO Build succeeded.
@REM EndLocal