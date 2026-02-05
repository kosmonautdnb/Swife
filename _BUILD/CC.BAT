@echo off
@echo .CC: "%1" : "R:\%2"
rem wcc386.exe -5r -fp5 -d3 -d+ -mc -zpw -zq -zp=1 %1 -foR:\%2 >> R:\LOGS.LOG
set DOS4G=quiet
if %DJGPPCOMPILE%==0 wcl386.exe /c /cc++ /5r /xr /fp3 /fp5 /otnlrm /mf /zpw /zq /zp1 %1 -foR:\%2 >> R:\LOGS.LOG
if %DJGPPCOMPILE%==1 gpp.exe -c -O3 -ISRC\STL -ISRC\IMGUI -oR:\%2 %1 >> R:\LOGS.LOG
rem @echo file R:\%2 >> R:\LINK.LNK