rem call c:\opwat20\setvars.bat
rem call c:\_wat_c_\setvars.bat
set INCLUDE=%INCLUDE%;SRC\STL;SRC\IMGUI;
_build\text.exe
@echo off
@echo Starting.. (needs RamDisk at R:)
rem @echo system dos4g > R:\LINK.LNK
@echo system pmodew > R:\LINK.LNK
@echo build errors > R:\LOGS.LOG
@echo -oR:\main.exe > R:\LINK2.LNK
_BUILD\FLTMECHK.exe !MAK R:\main.exe
if not ERRORLEVEL 255 call _BUILD\MAKEMK.BAT
SET DJGPPCOMPILE=0
call wmake.exe /f R:\makefile /e /s /k
if %DJGPPCOMPILE%==0 v R:\logs.log
SET DJGPPCOMPILE=0
