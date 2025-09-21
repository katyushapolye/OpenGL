@echo off
setlocal

set BUILD_DIR=build
set EXE_NAME=OpenGLApp.exe

:: --- Clean temporary build folder ---
if "%1%"=="clean" (
    echo [CLEAN] Removing temporary build folder...
    if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
    mkdir "%BUILD_DIR%"
    exit /b 0
)

:: --- Configure & Build ---
echo [BUILD] Configuring and building project...
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
pushd "%BUILD_DIR%" || (echo Failed to enter build folder & exit /b 1)

cmake .. -G "MinGW Makefiles"
if errorlevel 1 exit /b 1

cmake --build . --config Release
if errorlevel 1 exit /b 1

popd

:: --- Run ---
if "%1%"=="run" (
    echo [RUN] Running executable from bin...
    pushd "%~dp0bin" || (echo Failed to enter bin folder & exit /b 1)
    "%EXE_NAME%"
    popd
)
