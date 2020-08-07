rem mb: Travis-ci uses git-bash to execute scripts under Windows.
rem But vcvars needs to be called under cmd.exe, so this .bat file forces
rem git-bash to launch a cmd.exe to execute this.

rem this script requires nodecpp-checker and nodecpp-dezombiefy
rem to be already built in the path below

set PATH=%cd%\safe_memory\checker\build\travis\bin;%PATH%
set PATH=C:\ProgramData\chocolatey\lib\sccache\tools\sccache-0.2.12-x86_64-pc-windows-msvc;%PATH%

call "C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
@if ERRORLEVEL 1 exit /b %ERRORLEVEL%


rmdir /S /Q build\travis
mkdir build\travis
cd build\travis

cmake -DNODECPP_CHECK_AND_DZ_SAMPLES=ON -DCMAKE_BUILD_TYPE=Release -G Ninja ..\..
@if ERRORLEVEL 1 exit /b %ERRORLEVEL%

cmake --build .
@if ERRORLEVEL 1 exit /b %ERRORLEVEL%

cd ..\..

