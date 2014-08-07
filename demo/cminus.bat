"cminus.exe" %1.c
%cd%\bin\ml /c /coff /Cp  %1.asm
%cd%\bin\link /subsystem:console /entry:start %1.obj
del %1.obj
pause
