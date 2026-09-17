@echo off
rem Generate the Visual Studio solution + projects under opencod1\src via premake.
rem Usage:  generate.bat [action]     e.g.  generate.bat vs2019
rem Default action is vs2022.  Any premake VS action works: vs2022 vs2019 vs2017
rem vs2015 vs2013 vs2012 vs2010 vs2008 vs2005.
setlocal
cd /d "%~dp0"
set "ACTION=%~1"
if "%ACTION%"=="" set "ACTION=vs2022"
echo Generating %ACTION% project files...
tools\premake5.exe %ACTION%
if errorlevel 1 (
    echo.
    echo Generation failed.
    exit /b 1
)
echo Done.  Open opencod1\src\opencod1.sln in Visual Studio.
