@echo off
echo === 1. Konfiguriere CMake ===
:: Wir exportieren trotzdem die compile_commands.json (kann nicht schaden, falls wir später doch Ninja nehmen)
cmake -B cmake-build -DCMAKE_EXPORT_COMPILE_COMMANDS=1
if %errorlevel% neq 0 (
    echo Fehler beim Konfigurieren!
    exit /b %errorlevel%
)

echo.
echo === 2. Baue das Projekt ===
cmake --build cmake-build --config Debug
if %errorlevel% neq 0 (
    echo Fehler beim Bauen!
    exit /b %errorlevel%
)

echo.
echo === 3. Starte die Anwendung ===
cmake-build\ACR_artefacts\Debug\ACR.exe
