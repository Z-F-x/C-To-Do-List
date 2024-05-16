#include "theme.h"

BOOL darkTheme = FALSE;  // Define the global variable

void ApplyTheme(HWND hwnd, BOOL theme) {
    HBRUSH hBrush = CreateSolidBrush(theme ? RGB(45, 45, 48) : RGB(255, 255, 255));
    SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)hBrush);
    InvalidateRect(hwnd, NULL, TRUE);
}
