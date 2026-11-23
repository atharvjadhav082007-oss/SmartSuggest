@echo off
echo ===================================================
echo   Building Autocomplete ^& Spell Checker (C++17)
echo ===================================================

set CXX=g++
where g++ >nul 2>&1
if %errorlevel% neq 0 (
    set CXX=C:\MinGW\bin\g++.exe
)

echo Using compiler: %CXX%

set SRCS=src/Dictionary.cpp src/TrieNode.cpp src/Trie.cpp src/Levenshtein.cpp src/Autocomplete.cpp src/SpellChecker.cpp src/ParagraphChecker.cpp src/SpellingGame.cpp src/WhatsAppBot.cpp src/GrammarChecker.cpp

REM Compile the main application (Console Menu + HTTP Web Server)
echo [1/2] Compiling main application (app.exe)...
%CXX% -std=c++17 -Wall -Wextra -Iinclude %SRCS% src/server.cpp src/main.cpp -o app.exe -lws2_32

if %errorlevel% neq 0 (
    echo [ERROR] Build failed for main application!
    exit /b %errorlevel%
)

REM Compile unit test suite
echo [2/2] Compiling test suite (test_runner.exe)...
%CXX% -std=c++17 -Wall -Wextra -Iinclude %SRCS% src/test_cases.cpp -o test_runner.exe -lws2_32

if %errorlevel% neq 0 (
    echo [ERROR] Build failed for test suite!
    exit /b %errorlevel%
)

echo ===================================================
echo   Build Successful!
echo ===================================================
echo - Run Console / Server:  app.exe
echo - Run Web Server only:   app.exe --server
echo - Run Unit Test Suite:   test_runner.exe
echo ===================================================
