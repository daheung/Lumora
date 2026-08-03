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
SET includeFlags=-Isrc -I%VULKAN_SDK%\Include
SET linkerFlags=-Iuser32 -lvulkan-1 -L%VULKAN_SDK%\Lib
SET defines=-D_DEBUG -DLUMORA_EXPORT -D_CRT_SECURE_NO_WARNINGS

ECHO "Building %assembly%..."
clang %cFileNames% %compilerFlags% -o ../bin/%assembly%.dll %defines% %includeFlags% %linkerFlags%