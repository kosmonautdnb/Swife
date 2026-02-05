@echo build errors > LOGS.LOG
wcl386.exe /ms /fe=_build\genmak.exe _build\genmak.c >> LOGS.LOG
del genmak.obj
wcl386.exe /ms /fe=_build\fltmechk.exe _build\fltmechk.c >> LOGS.LOG
del fltmechk.obj
wcl386.exe /cc++ /ms /fe=_build\prepend.exe _build\prepend.c >> LOGS.LOG
del prepend.obj
wcl386.exe /ms /fe=_build\text.exe _build\text.c >> LOGS.LOG
del text.obj
