REM Build script for Engine
@ECHO OFF
SetLocal EnableDelayedExpansion

REM Get a list of all the .c files.
SET cFileNames=
FOR /R %%f  IN (*.c) DO (
    SET cFileNames=!cFileNames! "%%f"
)

REM echo "Files:" %cFileNames%

SET assembly=engine
SET compilerFlags=-g -shared -Wvarargs -Wall -Werror
REM -Wall -Werror
SET includeFlags=-ISource -ISource/Core -ISource/Renderer -I%VULKAN_SDK%\Include
SET linkerFlags=-Iuser32 -lvulkan-1 -L%VULKAN_SDK%\Lib
SET defines=-D_DEBUG -DLUMORA_EXPORT -D_CRT_SECURE_NO_WARNINGS

ECHO "Building %assembly%..."
clang %cFileNames% %compilerFlags% -o ../bin/%assembly%.dll %defines% %includeFlags% %linkerFlags%

@REM REM Build script for Engine
@REM @ECHO OFF
@REM SetLocal EnableDelayedExpansion

@REM REM Get a list of all the .c files.
@REM SET sourceFileNames=
@REM FOR /R %%f IN (*.c) DO (
@REM     SET sourceFileNames=!sourceFileNames! "%%f"
@REM )

@REM FOR /R %%f IN (*.cpp) DO (
@REM     SET sourceFileNames=!sourceFileNames! "%%f"
@REM )

@REM SET assembly=engine
@REM SET compilerFlags=-g -shared -Wvarargs -Wall -Werror
@REM SET includeFlags=-Isrc -I"%VULKAN_SDK%\Include"
@REM SET linkerFlags=-L"%VULKAN_SDK%\Lib" -luser32 -lvulkan-1 -Xlinker /IMPLIB:..\bin\engine.lib
@REM SET defines=-D_DEBUG -DLUMORA_EXPORT -D_CRT_SECURE_NO_WARNINGS

@REM ECHO Building %assembly%...
@REM clang++ %compilerFlags% ^
@REM     %defines% ^
@REM     %includeFlags% ^
@REM     -x c++ %sourceFileNames% ^
@REM     -o "..\bin\%assembly%.dll" ^
@REM     %linkerFlags%

@REM IF %ERRORLEVEL% NEQ 0 (
@REM     ECHO Build failed.
@REM     EXIT /B %ERRORLEVEL%
@REM )

@REM ECHO Build succeeded.
@REM EndLocal