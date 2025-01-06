#include "util.h"

#include <STDIO.H>

BOOL WriteMemory(LPVOID destination, LPVOID source, size_t length, LPVOID oldData)
{
  DWORD oldProtec;

  if (VirtualProtect(destination, length, PAGE_EXECUTE_READWRITE, &oldProtec)) {
    // Read data out if requested
    if (oldData) {
      memcpy(oldData, destination, length);
    }

    // Write data if provided
    if (source) {
      memcpy(destination, source, length);
    }

    // Restore original protection
    VirtualProtect(destination, length, oldProtec, &oldProtec);

    return TRUE;
  } else {
    return FALSE;
  }
}

LPVOID OverwriteImport(LPVOID imageBase, LPCSTR overrideFunction, LPVOID override)
{
  // Get DOS header
  PIMAGE_DOS_HEADER dosHeaders = (PIMAGE_DOS_HEADER)imageBase;

  // Retrieve NT header
  PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((UINT_PTR)imageBase + dosHeaders->e_lfanew);

  // Retrieve imports directory
  IMAGE_DATA_DIRECTORY importsDirectory = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];

  // Retrieve import descriptor
  PIMAGE_IMPORT_DESCRIPTOR importDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)((UINT_PTR)imageBase + importsDirectory.VirtualAddress);

  while (importDescriptor->Name != NULL) {
    LPCSTR libraryName = (LPCSTR)importDescriptor->Name + (UINT_PTR)imageBase;
    HMODULE library = LoadLibraryA(libraryName);

    if (library) {
      PIMAGE_THUNK_DATA originalFirstThunk = (PIMAGE_THUNK_DATA)((UINT_PTR)imageBase + importDescriptor->OriginalFirstThunk);
      PIMAGE_THUNK_DATA firstThunk = (PIMAGE_THUNK_DATA)((UINT_PTR)imageBase + importDescriptor->FirstThunk);

      while (originalFirstThunk->u1.AddressOfData != NULL) {
        PIMAGE_IMPORT_BY_NAME functionName = (PIMAGE_IMPORT_BY_NAME)((UINT_PTR)imageBase + (UINT_PTR)originalFirstThunk->u1.AddressOfData);

        if (!strcmp((const char*)functionName->Name, overrideFunction)) {
          // Write to memory
          LPVOID originalFunction;
          WriteMemory(&firstThunk->u1.Function, &override, sizeof(LPVOID), &originalFunction);

          // Return original function and end loop here
          printf("Hooked %s\n", overrideFunction);
          return originalFunction;
        }
        ++originalFirstThunk;
        ++firstThunk;
      }
    }

    importDescriptor++;
  }

  return NULL;
}
