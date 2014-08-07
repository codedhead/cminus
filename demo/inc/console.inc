  ; ===========================================
  ; console.inc
  ; this is modified from MASM32
  ; ===========================================

IFNDEF CONSOLE_INC
CONSOLE_INC equ <1>

NULL                                 equ 0
INVALID_HANDLE_VALUE                 equ -1
STD_INPUT_HANDLE                     equ -10
STD_OUTPUT_HANDLE                    equ -11
STD_ERROR_HANDLE                     equ -12

GetStdHandle PROTO :DWORD

ReadConsoleA PROTO :DWORD,:DWORD,:DWORD,:DWORD,:DWORD
ReadConsole equ <ReadConsoleA>

WriteConsoleA PROTO :DWORD,:DWORD,:DWORD,:DWORD,:DWORD
WriteConsole equ <WriteConsoleA>

ExitProcess PROTO :DWORD

ELSE
echo -----------------------------------------
echo WARNING Duplicate include file kernel32.inc
echo -----------------------------------------
ENDIF