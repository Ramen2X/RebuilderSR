#ifndef HOOKS_H
#define HOOKS_H

#include <cstdio>

#include <WINDOWS.H>

HWND WINAPI InterceptCreateWindowExA(
  DWORD          dwExStyle,
  LPCSTR         lpClassName,
  LPCSTR         lpWindowName,
  DWORD          dwStyle,
  int            X,
  int            Y,
  int            nWidth,
  int            nHeight,
  HWND           hWndParent,
  HMENU          hMenu,
  HINSTANCE      hInstance,
  LPVOID         lpParam
);



LONG
APIENTRY
InterceptRegQueryValueExA (
    HKEY hKey,
    LPCSTR lpValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE lpData,
    LPDWORD lpcbData
);

VOID WINAPI InterceptSleep(DWORD dwMilliseconds);

#endif // HOOKS_H
