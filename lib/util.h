#ifndef UTIL_H
#define UTIL_H

#include <WINDOWS.H>

BOOL WriteMemory(LPVOID destination, LPVOID source, size_t length, LPVOID oldData = NULL);

LPVOID OverwriteImport(LPVOID imageBase, LPCSTR overrideFunction, LPVOID override);

#endif // UTIL_H
