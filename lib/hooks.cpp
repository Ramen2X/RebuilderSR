#include "hooks.h"

#include <MAP>
#include <MATH.H>
#include <STDIO.H>
#include <VECTOR>

#include "../cmn/path.h"
#include "config.h"
#include "util.h"

HWND isleWindow = NULL;

HWND WINAPI InterceptCreateWindowExA(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
  // If full screen, hide the titlebar
  if (config.GetInt(_T("FullScreen"))) {
    dwStyle = WS_POPUP;
    dwExStyle = 0;
  }

  return CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

void ReturnRegistryYESNOFromBool(LPBYTE lpData, BOOL value)
{
  strcpy((char*)lpData, value ? "YES" : "NO");
}

LONG WINAPI InterceptRegQueryValueExA(HKEY hKey, LPCSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
{
  if (!strcmp(lpValueName, "Music")) {

    // Music option
    ReturnRegistryYESNOFromBool(lpData, config.GetInt(_T("MusicToggle"), 1));
    return ERROR_SUCCESS;

  } else if (!strcmp(lpValueName, "UseJoystick")) {

    ReturnRegistryYESNOFromBool(lpData, config.GetInt(_T("UseJoystick"), 0));
    return ERROR_SUCCESS;

  } else if (!strcmp(lpValueName, "Full Screen")) {

    ReturnRegistryYESNOFromBool(lpData, config.GetInt(_T("FullScreen"), 1));
    return ERROR_SUCCESS;

  } else if (!strcmp(lpValueName, "Draw Cursor")) {

    ReturnRegistryYESNOFromBool(lpData, config.GetInt(_T("DrawCursor"), 0));
    return ERROR_SUCCESS;

  } else if (!strcmp(lpValueName, "Flip Surfaces")) {

    ReturnRegistryYESNOFromBool(lpData, config.GetInt(_T("FlipSurfaces"), 0));
    return ERROR_SUCCESS;

  } else if (!strcmp(lpValueName, "Back Buffers in Video RAM")) {

    ReturnRegistryYESNOFromBool(lpData, config.GetInt(_T("BackBuffersInVRAM"), 0));
    return ERROR_SUCCESS;

  } else if (!strcmp(lpValueName, "3D Device ID")) {

    std::string dev_id = config.GetString("D3DDeviceID");
    strcpy((char*)lpData, dev_id.c_str());
    return ERROR_SUCCESS;

  } else if (!strcmp(lpValueName, "3D Device Name")) {

    std::string dev_name = config.GetString("D3DDevice");
    strcpy((char*)lpData, dev_name.c_str());
    return ERROR_SUCCESS;

  } else if (!strcmp(lpValueName, "diskpath") || !strcmp(lpValueName, "cdpath")) {

    // Pass through

  } else {

    printf("Passed through requested registry key \"%s\"\n", lpValueName);

  }

  // Pass these through
  return RegQueryValueExA(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
}

void WINAPI InterceptSleep(DWORD dwMilliseconds)
{
  // If uncapped, do nothing. Otherwise pass through as normal.
  if (config.GetString(_T("FPSLimit")) != "Uncapped") {
    Sleep(dwMilliseconds);
  }
}

