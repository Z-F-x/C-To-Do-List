// bitmap.h
#ifndef BITMAP_H // Prevent multiple inclusions
#define BITMAP_H

#include <windows.h> // Include Windows.h for types used in the function prototype

HBITMAP CreateCheckboxBitmap(HDC hdc, BOOL checked);

#endif // BITMAP_H