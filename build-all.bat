@ECHO OFF
REM Build Everything

ECHO "Building Engine..."

PUSHD Engine
CALL build.bat
POPD
IF %ERRORLEVEL% NEQ 0 (echo Error: %ERRORLEVEL% %% exit)

PUSHD Testbed
CALL build.bat
POPD
IF %ERRORLEVEL% NEQ 0 (echo Error: %ERRORLEVEL% %% exit)

ECHO "All builds completed successfully."