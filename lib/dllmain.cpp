#include <IOSTREAM>
#include <TCHAR.H>
#include <VECTOR>
#include <WINDOWS.H>

#include "config.h"
#include "hooks.h"
#include "util.h"

__declspec(dllexport) DWORD WINAPI Patch()
{
  if (!config.Load()) {
    MessageBoxA(0, "Failed to find RebuilderSR configuration. The game will start with its vanilla settings.", 0, 0);
    return 1;
  }

  // Hook import address table
  LPVOID exeBase = GetModuleHandle(TEXT("ISLE.EXE"));
  LPVOID dllBase = GetModuleHandle(TEXT("LEGO1.DLL"));

  // Redirect various imports
  OverwriteImport(exeBase, "CreateWindowExA", (LPVOID)InterceptCreateWindowExA);
  OverwriteImport(exeBase, "RegQueryValueExA", (LPVOID)InterceptRegQueryValueExA);
  OverwriteImport(dllBase, "Sleep", (LPVOID)InterceptSleep);
  OverwriteImport(exeBase, "Sleep", (LPVOID)InterceptSleep);

  // Flip surfaces is incompatible with full screen, if these options are set, warn the user
  if (config.GetInt(_T("FlipSurfaces")) && !config.GetInt(_T("FullScreen"))) {
    if (MessageBoxA(0, "The setting 'Flip Video Memory Pages' is incompatible with LEGO Island's windowed mode. "
                       "LEGO Island will likely fail to start up unless you disable 'Flip Video Memory Pages' "
                       "or run in full screen mode. Do you wish to continue?", "Warning", MB_YESNO) == IDNO) {
      TerminateProcess(GetCurrentProcess(), 0);
    }
  }

  // FPS Cap
  std::string fps_behavior = config.GetString(_T("FPSLimit"));
  if (fps_behavior != "Default") {
    UINT32 frame_delta = 1000.0f / config.GetFloat(_T("CustomFPS"));
    WriteMemory((char*)exeBase+0x10B4, &frame_delta, sizeof(UINT32));
  }

  return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
  switch (ul_reason_for_call) {
  case DLL_PROCESS_ATTACH:
    // Allocate console
    AllocConsole();

    // Direct stdin/stderr/stdout to console
    _tfreopen(TEXT("CONIN$"), TEXT("r"), stdin);
    _tfreopen(TEXT("CONOUT$"), TEXT("w"), stderr);
    _tfreopen(TEXT("CONOUT$"), TEXT("w"), stdout);

    // Print success line
    printf("Injected successfully\n");

    Patch();
    break;
  }

  return TRUE;
}
