@echo off
setlocal
chcp 65001 >nul

set "ROOT_DIR=%~dp0"
for %%I in ("%ROOT_DIR%.") do set "ROOT_DIR=%%~fI"

set "BUILD_CONFIG=Release"
if /I "%~1"=="debug" set "BUILD_CONFIG=Debug"
if /I "%~1"=="release" set "BUILD_CONFIG=Release"
if /I "%~1"=="all" set "BUILD_CONFIG=All"

set "BUILD_DIR=%ROOT_DIR%\build\PixelForgeSymbols\win64"
set "INSTALL_DIR=%ROOT_DIR%\build\PixelForge\win64"

echo ========================================
echo PixelForge build
echo ========================================
echo Root:    %ROOT_DIR%
echo Build:   %BUILD_DIR%
echo Install: %INSTALL_DIR%
echo Config:  %BUILD_CONFIG%
echo.

if /I "%~1"=="clean" (
    if exist "%BUILD_DIR%" rmdir /S /Q "%BUILD_DIR%"
    if exist "%INSTALL_DIR%" rmdir /S /Q "%INSTALL_DIR%"
    echo Clean complete.
    exit /b 0
)

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

echo ========================================
echo Configure
echo ========================================
cmake -S "%ROOT_DIR%" -B "%BUILD_DIR%" -G "Visual Studio 17 2022" -A x64 -DCMAKE_INSTALL_PREFIX="%INSTALL_DIR%"

if errorlevel 1 (
    echo CMake configuration failed.
    exit /b 1
)

echo.
echo ========================================
echo Build
echo ========================================
cmake --build "%BUILD_DIR%" --config %BUILD_CONFIG% -- /m:8

if errorlevel 1 (
    echo Build failed.
    exit /b 1
)

echo.
echo ========================================
echo Install
echo ========================================
cmake --install "%BUILD_DIR%" --config %BUILD_CONFIG%

if errorlevel 1 (
    echo Install failed.
    exit /b 1
)

echo.
echo ========================================
echo BUILD SUCCESSFUL
echo ========================================
echo Output: %INSTALL_DIR%\bin\PixelForge.exe
