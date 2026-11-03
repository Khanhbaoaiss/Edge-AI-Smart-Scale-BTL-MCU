@echo off
chcp 65001 >nul 2>&1
title Smart Scale POS Launcher
cd /d "%~dp0"

echo ==============================================================================
echo        SMART SCALE POS - EDGE AI SMART SCALE LAUNCHER
echo ==============================================================================
echo.

if exist "FruitTinyML\Run_POS_App.bat" (
    call "FruitTinyML\Run_POS_App.bat" %*
) else if exist "scripts\pos_desktop_app.py" (
    python scripts\pos_desktop_app.py %*
) else (
    echo [ERROR] Cannot locate POS application launcher!
    pause
    exit /b 1
)
