@echo off
setlocal EnableDelayedExpansion

echo ========================================
echo   WizzMania Client Build Script
echo ========================================
echo.

REM Check if Qt6 is in PATH
where /q qmake
if errorlevel 1 (
    echo ERROR: Qt6 not found in PATH!
    echo.
    echo Please add Qt 6.10 to your PATH, for example:
    echo   C:\Qt\6.10.0\msvc2022_64\bin
    echo.
    echo Or run this script from Qt Creator's terminal
    pause
    exit /b 1
)

echo Qt6 found in PATH
qmake --version
echo.

REM Navigate to client directory
cd /d "%~dp0client"

REM Create build directory
if not exist "build" mkdir build
cd build

echo ========================================
echo Step 1: Configuring with CMake
echo ========================================
cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 (
    echo.
    echo CMake configuration failed!
    echo Try using Qt Creator or check your Qt installation
    pause
    exit /b 1
)

echo.
echo ========================================
echo Step 2: Building Client
echo ========================================
cmake --build . --config Release
if errorlevel 1 (
    echo.
    echo Build failed!
    pause
    exit /b 1
)

echo.
echo ========================================
echo Build Successful!
echo ========================================
echo Executable: client\build\wizzmania-client.exe
echo.
echo To run: cd client\build ^&^& wizzmania-client.exe
echo.

pause